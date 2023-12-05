// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* TENOR_BASE_URL = "https://tenor.googleapis.com/v2/";

}

Tenor::Tenor(QObject* parent) :
    QObject(parent),
    mApiKey(TENOR_API_KEY),
    mClientKey("com.gmail.mfnboer.skywalker")
{
    QLocale locale;
    mLocale = QString("%1_%2").arg(
        QLocale::languageToCode(locale.language()),
        QLocale::territoryToCode(locale.territory()));
    qDebug() << "Locale:" << mLocale;

    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
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
    // We use the MP4 formats as those are much smaller than GIF
    Params params{{"q", query},
                  {"media_filter", "mp4,nanomp4,gifpreview"},
                  {"contentfilter", "medium"}};

    if (!pos.isEmpty())
        params.append({"pos", pos});

    QNetworkRequest request(buildUrl("search", params));
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]{ searchGifsFinished(reply); });

    connect(reply, &QNetworkReply::errorOccurred, this, [reply](auto errCode){
        qWarning() << "Search error:" << reply->request().url() << "error:" <<
                errCode << reply->errorString();
    });

    connect(reply, &QNetworkReply::sslErrors, this, [reply]{
        qWarning() << "Search SSL error:" <<  reply->request().url();
    });
}

void Tenor::getCategories()
{
    getCategories("featured", mCachedFeaturedCategories,
                  [this]{ getCategories("trending", mCachedTrendingCategories); });
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
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, &categoryList, getNext]{
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

void Tenor::searchGifsFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Search failed:" << reply->request().url() << "error:" <<
                reply->error() << reply->errorString();
        emit searchGifsFailed();
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const ATProto::XJsonObject xjson(json.object());

    const auto results = xjson.getOptionalArray("results");

    if (!results)
    {
        qWarning("results are missing");
        emit searchGifsFailed();
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
            const auto mediaFormatsJson = resultXJson.getRequiredObject("media_formats");
            const ATProto::XJsonObject mediaFormatsXJson(mediaFormatsJson);
            const auto mp4Json = mediaFormatsXJson.getRequiredObject("mp4");
            const auto mp4Format = mediaFormatFromJson(mp4Json);
            const auto nanoMp4Json = mediaFormatsXJson.getRequiredObject("nanomp4");
            const auto nanoMp4Format = mediaFormatFromJson(nanoMp4Json);
            const auto gifPreviewJson = mediaFormatsXJson.getRequiredObject("gifpreview");
            const auto gifPreviewFormat = mediaFormatFromJson(gifPreviewJson);

            const TenorGif tenorGif(id, description,
                                    mp4Format.mUrl, mp4Format.mSize,
                                    nanoMp4Format.mUrl, nanoMp4Format.mSize,
                                    gifPreviewFormat.mUrl, gifPreviewFormat.mSize);

            tenorGifList.append(tenorGif);
        }
        catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid JSON:" << e.msg();
            continue;
        }
    }

    const auto next = xjson.getOptionalString("next", "");
    emit searchGifsResult(tenorGifList, next);
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
    const ATProto::XJsonObject xjson(json.object());

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
