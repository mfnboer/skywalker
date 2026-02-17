// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* TENOR_BASE_URL = "https://tenor.googleapis.com/v2/";
constexpr char const* MEDIA_FILTER = "tinygif,gif,gifpreview";
constexpr char const* CONTENT_FILTER = "medium";
constexpr int MAX_RECENT_GIFS = 50;

}

Tenor::Tenor(QObject* parent) :
    WrappedSkywalker(parent),
    Presence(),
    mApiKey(TENOR_API_KEY),
    mClientKey("com.gmail.mfnboer.skywalker"),
    mOverviewModel(mWidth, mSpacing, this),
    mNetwork(new QNetworkAccessManager(this))
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
    QUrl url(TENOR_BASE_URL + endpoint);
    QUrlQuery query;
    query.addQueryItem("key", QUrl::toPercentEncoding(mApiKey));
    query.addQueryItem("client_key", QUrl::toPercentEncoding(mClientKey));

    if (endpoint != "posts")
        query.addQueryItem("locale", QUrl::toPercentEncoding(mLocale));

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
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
    Params params{{"ids", gifIdList.join(',')},
                  {"media_filter", MEDIA_FILTER}};

    setSearchInProgress(true);
    QNetworkRequest request(buildUrl("posts", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        setSearchInProgress(false);
        searchGifsFinished(reply, "");
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, presence=getPresence(), reply](auto errCode){
        if (!presence)
            return;

        setSearchInProgress(false);
        qWarning() << "Posts error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

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

    Params params{{"q", query},
                  {"media_filter", MEDIA_FILTER},
                  {"contentfilter", CONTENT_FILTER}};

    if (!pos.isEmpty())
        params.append({"pos", pos});

    setSearchInProgress(true);
    QNetworkRequest request(buildUrl("search", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply, query]{
        if (!presence)
            return;

        setSearchInProgress(false);
        searchGifsFinished(reply, query);
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, presence=getPresence(), reply](auto errCode){
        if (!presence)
            return;

        setSearchInProgress(false);
        qWarning() << "Search error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

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
    QNetworkRequest request(buildUrl("categories", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply, &categoryList, getNext]{
        if (!presence)
            return;

        categoriesFinished(reply, categoryList);

        if (getNext)
            getNext();
        else
            getRecentCategory();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [reply](auto errCode){
        qWarning() << "Categories error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [reply]{
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

    Params params{{"ids", gifIds.front()},
                  {"media_filter", MEDIA_FILTER}};

    QNetworkRequest request(buildUrl("posts", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        setRecentCategory(reply);
        allCategoriesRetrieved();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, presence=getPresence(), reply](auto errCode){
        if (!presence)
            return;

        qWarning() << "Posts error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
        allCategoriesRetrieved();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

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
    qDebug() << id << description;

    const auto mediaFormatsJson = resultXJson.getRequiredJsonObject("media_formats");
    const ATProto::XJsonObject mediaFormatsXJson(mediaFormatsJson);
    const auto smallJson = mediaFormatsXJson.getRequiredJsonObject("tinygif");
    const auto smallFormat = mediaFormatFromJson(smallJson);
    const auto imageJson = mediaFormatsXJson.getRequiredJsonObject("gifpreview");
    const auto imageFormat = mediaFormatFromJson(imageJson);

    // The mediumgif format seems to be better as it is smaller without
    // much loss of quality. However Bluesky only plays the gif format.
    const auto gifJson = mediaFormatsXJson.getRequiredJsonObject("gif");
    const auto gifFormat =  mediaFormatFromJson(gifJson);

    const TenorGif tenorGif(id, description, query,
                            gifFormat.mUrl, gifFormat.mSize,
                            smallFormat.mUrl, smallFormat.mSize,
                            imageFormat.mUrl, imageFormat.mSize);

    return tenorGif;
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

    QNetworkRequest request(buildUrl("registershare", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply]{
        if (reply->error() == QNetworkReply::NoError)
        {
            qDebug() << "Register Share OK";
        }
        else
        {
            qWarning() << "Register Share failed:" << reply->request().url() << "error:" <<
                reply->error() << reply->errorString();
        }
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [reply](auto errCode){
        qWarning() << "Register Share error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [reply]{
        qWarning() << "Register Share SSL error:" <<  reply->request().url();
    });
}

void Tenor::addRecentGif(const TenorGif& gif)
{
    const QString did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList gifIds = settings->getRecentGifs(did);

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
    settings->setRecentGifs(did, gifIds);
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
    auto* settings = mSkywalker->getUserSettings();
    QStringList gifIds = settings->getRecentGifs(did);
    return gifIds;
}

}
