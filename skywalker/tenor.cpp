// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* TENOR_BASE_URL = "https://tenor.googleapis.com/v2/";

}

QNetworkAccessManager Tenor::sNetwork;

Tenor::Tenor(QObject* parent) :
    QObject(parent),
    Presence(),
    mApiKey(TENOR_API_KEY),
    mClientKey("com.gmail.mfnboer.skywalker"),
    mOverviewModel(mWidth, mSpacing, this)
{
    QLocale locale;
    mLocale = QString("%1_%2").arg(
        QLocale::languageToCode(locale.language()),
        QLocale::territoryToCode(locale.territory()));
    qDebug() << "Locale:" << mLocale;

    sNetwork.setAutoDeleteReplies(true);
    sNetwork.setTransferTimeout(15000);
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
    query.addQueryItem("locale", QUrl::toPercentEncoding(mLocale));

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
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
                  {"media_filter", "tinygif,gifpreview"},
                  {"contentfilter", "medium"}};

    if (!pos.isEmpty())
        params.append({"pos", pos});

    setSearchInProgress(true);
    QNetworkRequest request(buildUrl("search", params));
    QNetworkReply* reply = sNetwork.get(request);

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
    Q_ASSERT(!mQuery.isEmpty());

    if (mQuery.isEmpty())
    {
        qWarning() << "No previous query available";
        return;
    }

    if (mNextPos.isEmpty())
    {
        qDebug() << "End of feed";
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
            allCategoriesRetrieved();
        }

        return;
    }

    Params params{{"type", type}, {"contentfilter", "medium"}};
    QNetworkRequest request(buildUrl("categories", params));
    QNetworkReply* reply = sNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply, &categoryList, getNext]{
        if (!presence)
            return;

        categoriesFinished(reply, categoryList);

        if (getNext)
            getNext();
        else
            allCategoriesRetrieved();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [reply](auto errCode){
        qWarning() << "Categories error:" << reply->request().url() << "error:" <<
            errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [reply]{
        qWarning() << "Categories SSL error:" <<  reply->request().url();
    });
}

void Tenor::registerShare(const TenorGif& gif)
{
    // ID will be empty if the gif was save as part of a draft post
    if (gif.getId().isEmpty())
        return;

    qDebug() << "Register share:" << gif.getDescription() <<
            "id:" << gif.getId() << "q:" << gif.getSearchTerm();

    Params params{{"id", gif.getId()}, {"q", gif.getSearchTerm()}};
    QNetworkRequest request(buildUrl("registershare", params));
    QNetworkReply* reply = sNetwork.get(request);

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
        if (!resultElem.isObject())
        {
            qWarning() << "Non-object element in results";
            continue;
        }

        try {
            const auto resultJson = resultElem.toObject();
            const ATProto::XJsonObject resultXJson(resultJson);
            const auto id = resultXJson.getRequiredString("id");
            const auto description = resultXJson.getOptionalString("content_description", "");
            qDebug() << id << description;

            // Would be nicer to add URL to an MP4 or GIF directly.
            // However the itemurl is compatible with the official Bluesky app.
            const auto gifViewUrl = resultXJson.getRequiredString("itemurl");

            const auto mediaFormatsJson = resultXJson.getRequiredJsonObject("media_formats");
            const ATProto::XJsonObject mediaFormatsXJson(mediaFormatsJson);
            const auto smallJson = mediaFormatsXJson.getRequiredJsonObject("tinygif");
            const auto smallFormat = mediaFormatFromJson(smallJson);
            const auto imageJson = mediaFormatsXJson.getRequiredJsonObject("gifpreview");
            const auto imageFormat = mediaFormatFromJson(imageJson);

            const TenorGif tenorGif(id, description, query, gifViewUrl,
                                    smallFormat.mUrl, smallFormat.mSize,
                                    imageFormat.mUrl, imageFormat.mSize);

            tenorGifList.append(tenorGif);
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

}
