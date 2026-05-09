// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* KLIPY_BASE_URL = "https://api.klipy.com/v2/";
constexpr char const* TENOR_BASE_URL = "https://tenor.googleapis.com/v2/";
constexpr char const* MEDIA_FILTER = "tinygif,gif,gifpreview,mp4,webm";
constexpr char const* CONTENT_FILTER = "medium";
constexpr int MAX_RECENT_GIFS = 50;

}

Tenor::Tenor(QObject* parent) :
    WrappedSkywalker(parent),
    WebServiceBase(TENOR_BASE_URL, new QNetworkAccessManager(this)),
    mApiKey(TENOR_API_KEY),
    mClientKey("com.gmail.mfnboer.skywalker"),
    mOverviewModel(mWidth, mSpacing, this)
{
    QLocale locale;
    mLocale = QString("%1_%2").arg(
        QLocale::languageToCode(locale.language()),
        QLocale::territoryToCode(locale.territory()));
    qDebug() << "Locale:" << mLocale;

    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
}

void Tenor::setWidth(int width)
{
    if (width == mWidth)
        return;

    mWidth = width;
    mOverviewModel.setMaxRowWidth(mWidth);
    emit widthChanged();
}

void Tenor::setSpacing(int spacing)
{
    if (spacing == mSpacing)
        return;

    mSpacing = spacing;
    mOverviewModel.setSpacing(mSpacing);
    emit spacingChanged();
}

void Tenor::setSearchInProgress(bool inProgress)
{
    if (inProgress == mSearchInProgress)
        return;

    mSearchInProgress = inProgress;
    emit searchInProgressChanged();
}

QUrl Tenor::buildUrl(const QString& endpoint, const Params& params) const
{
    Params p(params);
    p.push_back({"key", QUrl::toPercentEncoding(mApiKey)});

    if (!mClientKey.isEmpty())
        p.push_back({"client_key", QUrl::toPercentEncoding(mClientKey)});

    if (endpoint != "posts")
        p.push_back({"locale", QUrl::toPercentEncoding(mLocale)});

    return WebServiceBase::buildUrl(endpoint, p);
}

void Tenor::searchRecentGifs()
{
    if (mSearchInProgress)
    {
        qWarning() << "Search already in progress";
        return;
    }

    mNextPos.clear();
    mOverviewModel.clear();
    mQuery.clear();

    const QStringList gifIdList = getRecentGifs();

    if (gifIdList.empty())
        return;

    Q_ASSERT(gifIdList.size() <= 50); // maximum allowed by Tenor
    // NOTE: gifpreview and webm is not accepted in the filter by KLIPY
    // Params params{{"ids", gifIdList.join(',')},
    //               {"media_filter", MEDIA_FILTER}};
    Params params{{"ids", gifIdList.join(',')}};

    setSearchInProgress(true);
    sendRequest("posts", params,
        [this](QNetworkReply* reply){
            setSearchInProgress(false);
            searchGifsFinished(reply, "");
        },
        [this](QNetworkReply* reply, auto errCode){
            setSearchInProgress(false);
            qWarning() << "Posts error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
        },
        [this](QNetworkReply* reply){
            setSearchInProgress(false);
            qWarning() << "Posts SSL error:" <<  reply->request().url();
        });
}

void Tenor::searchGifs(const QString& query, const QString& pos)
{
    if (mSearchInProgress)
    {
        qWarning() << "Search already in progress";
        return;
    }

    if (pos.isEmpty())
    {
        mNextPos.clear();
        mOverviewModel.clear();
        mQuery = query;
    }
    else
    {
        Q_ASSERT(query == mQuery);
    }

    // NOTE: here KLIPY accepts gifpreview and webm in the filter ???
    Params params{{"q", query},
                  {"media_filter", MEDIA_FILTER},
                  {"contentfilter", CONTENT_FILTER}};

    if (!pos.isEmpty())
        params.append({"pos", pos});

    setSearchInProgress(true);
    sendRequest("search", params,
        [this, query](QNetworkReply* reply){
            setSearchInProgress(false);
            searchGifsFinished(reply, query);
        },
        [this](QNetworkReply* reply, auto errCode){
            setSearchInProgress(false);
            qWarning() << "Search error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
        },
        [this](QNetworkReply* reply){
            setSearchInProgress(false);
            qWarning() << "Search SSL error:" <<  reply->request().url();
        });
}

void Tenor::getNextPage()
{
    if (mNextPos.isEmpty())
    {
        qDebug() << "End of feed";
        return;
    }

    Q_ASSERT(!mQuery.isEmpty());
    if (mQuery.isEmpty())
    {
        qWarning() << "No previous query available";
        return;
    }

    searchGifs(mQuery, mNextPos);
}

void Tenor::getCategories()
{
    // The trending categories seem not very valuable, and there are
    // more than enough featured categories.
    getCategories("featured", mCachedFeaturedCategories);
}

void Tenor::getCategories(const QString& type, TenorCategoryList& categoryList, const std::function<void()>& getNext)
{
    if (!categoryList.empty())
    {
        if (getNext)
            getNext();
        else {
            getRecentCategory();
        }

        return;
    }

    Params params{{"type", type}, {"contentfilter", CONTENT_FILTER}};
    sendRequest("categories", params,
        [this, &categoryList, getNext](QNetworkReply* reply){
            categoriesFinished(reply, categoryList);

            if (getNext)
                getNext();
            else
                getRecentCategory();
        },
        [](QNetworkReply* reply, auto errCode){
            qWarning() << "Categories error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
        },
        [](QNetworkReply* reply){
            qWarning() << "Categories SSL error:" <<  reply->request().url();
        });
}

void Tenor::getRecentCategory()
{
    if (mRecentCategory)
    {
        allCategoriesRetrieved();
        return;
    }

    const QStringList gifIds = getRecentGifs();

    if (gifIds.empty())
    {
        allCategoriesRetrieved();
        return;
    }

    // NOTE: gifpreview and webm are not accepted in the filter by KLIPY
    // Params params{{"ids", gifIds.front()},
    //               {"media_filter", MEDIA_FILTER}};
    Params params{{"ids", gifIds.front()}};

    sendRequest("posts", params,
        [this](QNetworkReply* reply){
            setRecentCategory(reply);
            allCategoriesRetrieved();
        },
        [this](QNetworkReply* reply, auto errCode){
            qWarning() << "Posts error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
            allCategoriesRetrieved();
        },
        [this](QNetworkReply* reply){
            qWarning() << "Posts SSL error:" <<  reply->request().url();
            allCategoriesRetrieved();
        });
}

void Tenor::setRecentCategory(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Get recent category failed:" << reply->request().url() << "error:" <<
            reply->error() << reply->errorString();
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);
    const auto results = xjson.getOptionalArray("results");

    if (!results)
    {
        qWarning("Failed to get recent category");
        return;
    }

    const auto& resultElem = results->first();

    try {
        const TenorGif gif = toTenorGif(resultElem, "");
        setRecentCategory(gif);
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
    }
}

void Tenor::setRecentCategory(const TenorGif& gif)
{
    mRecentCategory = TenorCategory(gif.getSmallUrl(), tr("Recently Used"), true);
}

TenorGif Tenor::toTenorGif(const QJsonValue& resultElem, const QString& query) const
{
    if (!resultElem.isObject())
        throw ATProto::InvalidJsonException("Non-object element");

    const auto resultJson = resultElem.toObject();
    const ATProto::XJsonObject resultXJson(resultJson);
    const auto id = resultXJson.getRequiredString("id");
    const auto description = resultXJson.getOptionalString("content_description", "");
    const auto title = resultXJson.getOptionalString("title", "");
    qDebug() << id << title << "description:" << description;

    const auto mediaFormatsJson = resultXJson.getRequiredJsonObject("media_formats");
    const ATProto::XJsonObject mediaFormatsXJson(mediaFormatsJson);
    const auto smallJson = mediaFormatsXJson.getRequiredJsonObject("tinygif");
    const auto smallFormat = mediaFormatFromJson(smallJson);
    const auto imageJson = mediaFormatsXJson.getRequiredJsonObject("gifpreview");
    const auto imageFormat = mediaFormatFromJson(imageJson);

    // The mediumgif format seems to be better as it is smaller without
    // much loss of quality. However Bluesky only plays the gif format.
    const auto gifJson = mediaFormatsXJson.getRequiredJsonObject("gif");
    const auto gifFormat = mediaFormatFromJson(gifJson);

    const auto mp4Json = mediaFormatsXJson.getOptionalJsonObject("mp4");
    const QString mp4Url = mp4Json ? mediaUrlFromJson(*mp4Json) : "";
    const auto webmJson = mediaFormatsXJson.getOptionalJsonObject("webm");
    const QString webmUrl = webmJson ? mediaUrlFromJson(*webmJson) : "";

    const TenorGif tenorGif(id, description.isEmpty() ? title : description, query,
                            gifFormat.mUrl, gifFormat.mSize,
                            smallFormat.mUrl, smallFormat.mSize,
                            imageFormat.mUrl, imageFormat.mSize,
                            mp4Url, webmUrl);

    return tenorGif;
}

void Tenor::setRecentGifs(const QString& did, const QStringList& gifIds)
{
    auto* settings = mSkywalker->getUserSettings();
    settings->setRecentGifs(did, gifIds);
}

QStringList Tenor::getRecentGifs(const QString& did) const
{
    auto* settings = mSkywalker->getUserSettings();
    return settings->getRecentGifs(did);
}

void Tenor::registerShare(const TenorGif& gif)
{
    // ID will be empty if the gif was saved as part of a draft post
    if (gif.getId().isEmpty())
        return;

    qDebug() << "Register share:" << gif.getDescription() <<
            "id:" << gif.getId() << "q:" << gif.getSearchTerm();

    Params params{{"id", gif.getId()}};

    if (!gif.getSearchTerm().isEmpty())
        params.push_back({"q", gif.getSearchTerm()});

    sendRequest("registershare", params,
        [](QNetworkReply* reply){
            if (reply->error() == QNetworkReply::NoError)
            {
                qDebug() << "Register Share OK";
            }
            else
            {
                qWarning() << "Register Share failed:" << reply->request().url() << "error:" <<
                    reply->error() << reply->errorString();
            }
        },
        [](QNetworkReply* reply, auto errCode){
            qWarning() << "Register Share error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
        },
        [](QNetworkReply* reply){
            qWarning() << "Register Share SSL error:" <<  reply->request().url();
        });
}

void Tenor::addRecentGif(const TenorGif& gif)
{
    const QString did = mSkywalker->getUserDid();
    QStringList gifIds = getRecentGifs(did);

    for (auto it = gifIds.cbegin(); it != gifIds.cend(); ++it)
    {
        const auto& recentGifId = *it;

        if (gif.getId() == recentGifId)
        {
            gifIds.erase(it);
            break;
        }
    }

    while (gifIds.size() >= MAX_RECENT_GIFS)
        gifIds.pop_back();

    gifIds.push_front(gif.getId());
    setRecentGifs(did, gifIds);
    setRecentCategory(gif);
}

void Tenor::searchGifsFinished(QNetworkReply* reply, const QString& query)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Search failed:" << reply->request().url() << "error:" <<
                reply->error() << reply->errorString();
        emit searchGifsFailed(reply->errorString());
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    const auto results = xjson.getOptionalArray("results");

    if (!results)
    {
        qDebug("no results");
        return;
    }

    TenorGifList tenorGifList;

    for (const auto& resultElem : *results)
    {
        try {
            const TenorGif gif = toTenorGif(resultElem, query);
            tenorGifList.append(gif);
        }
        catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid JSON:" << e.msg();
            continue;
        }
    }

    mNextPos = xjson.getOptionalString("next", "");
    mOverviewModel.addGifs(tenorGifList);
}

Tenor::MediaFormat Tenor::mediaFormatFromJson(const QJsonObject& json) const
{
    MediaFormat mediaFormat;
    const ATProto::XJsonObject xjson(json);
    mediaFormat.mUrl = xjson.getRequiredString("url");
    const auto dimsArray = xjson.getRequiredArray("dims");

    if (dimsArray.size() < 2)
        throw ATProto::InvalidJsonException(QString("Invalid size of dims: %1").arg(dimsArray.size()));

    mediaFormat.mSize.setWidth(dimsArray[0].toInt());
    mediaFormat.mSize.setHeight(dimsArray[1].toInt());

    if (mediaFormat.mSize.isEmpty())
        throw ATProto::InvalidJsonException("Invalid size");

    qDebug() << mediaFormat.mUrl << mediaFormat.mSize;
    return mediaFormat;
}

QString Tenor::mediaUrlFromJson(const QJsonObject& json) const
{
    const ATProto::XJsonObject xjson(json);
    return xjson.getRequiredString("url");
}

bool Tenor::categoriesFinished(QNetworkReply* reply, TenorCategoryList& categoryList)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Categories failed:" << reply->request().url() << "error:" <<
            reply->error() << reply->errorString();
        return false;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    const auto tags = xjson.getOptionalArray("tags");

    if (!tags)
    {
        qWarning("tags are missing");
        return false;
    }

    for (const auto& tag : *tags)
    {
        if (!tag.isObject())
        {
            qWarning() << "Non-object element in tags";
            continue;
        }

        const auto tagJson = tag.toObject();
        const auto tagXJson = ATProto::XJsonObject(tagJson);

        try {
            const auto searchTerm = tagXJson.getRequiredString("searchterm");
            const auto gifUrl = tagXJson.getRequiredString("image");
            qDebug() << "Category:" << searchTerm << gifUrl;
            const TenorCategory category(gifUrl, searchTerm);
            categoryList.append(category);
        }
        catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid JSON:" << e.msg();
            continue;
        }
    }

    return true;
}

void Tenor::allCategoriesRetrieved()
{
    TenorCategoryList categoryList;

    if (mRecentCategory)
        categoryList.push_back(*mRecentCategory);

    int i = 0;
    int j = 0;

    while (i < mCachedFeaturedCategories.size() || j < mCachedTrendingCategories.size())
    {
        if (i < mCachedFeaturedCategories.size())
            categoryList.append(mCachedFeaturedCategories[i++]);

        if (j < mCachedTrendingCategories.size())
            categoryList.append(mCachedTrendingCategories[j++]);
    }

    emit categories(categoryList);
}

QStringList Tenor::getRecentGifs()
{
    const QString did = mSkywalker->getUserDid();
    QStringList gifIds = getRecentGifs(did);
    return gifIds;
}

Klipy::Klipy(QObject* parent) : Tenor(parent)
{
    setBaseUrl(KLIPY_BASE_URL);
    setApiKey(KLIPY_API_KEY);
    setClientKey({});
}

TenorGif Klipy::toTenorGif(const QJsonValue& resultElem, const QString& query) const
{
    TenorGif gif = Tenor::toTenorGif(resultElem, query);
    gif.setGifProvider(QEnums::GIF_PROVIDER_KLIPY);
    return gif;
}

void Klipy::setRecentGifs(const QString& did, const QStringList& gifIds)
{
    auto* settings = mSkywalker->getUserSettings();
    settings->setRecentKlipyGifs(did, gifIds);
}

QStringList Klipy::getRecentGifs(const QString& did) const
{
    auto* settings = mSkywalker->getUserSettings();
    return settings->getRecentKlipyGifs(did);
}

}
