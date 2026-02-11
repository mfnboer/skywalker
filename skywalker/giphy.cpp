// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "giphy.h"
#include "giphy_gif.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* GIPHY_BASE_URL="https://api.giphy.com/v1/gifs/";

}

Giphy::Giphy(QObject* parent) :
    WrappedSkywalker(parent),
    Presence(),
    mApiKey(GIPHY_API_KEY),
    mOverviewModel(mWidth, mSpacing, this),
    mNetwork(new QNetworkAccessManager(this))
{
    QLocale locale;
    mLanguageCode = QLocale::languageToCode(locale.language());
    qDebug() << "Language code:" << mLanguageCode;

    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
}

void Giphy::setWidth(int width)
{
    if (width == mWidth)
        return;

    mWidth = width;
    mOverviewModel.setMaxRowWidth(mWidth);
    emit widthChanged();
}

void Giphy::setSpacing(int spacing)
{
    if (spacing == mSpacing)
        return;

    mSpacing = spacing;
    mOverviewModel.setSpacing(mSpacing);
    emit spacingChanged();
}

void Giphy::setSearchInProgress(bool inProgress)
{
    if (inProgress == mSearchInProgress)
        return;

    mSearchInProgress = inProgress;
    emit searchInProgressChanged();
}

void Giphy::getCategories()
{
    if (!mCachedCategories.empty())
    {
        allCategoriesRetrieved();
        return;
    }

    QNetworkRequest request(buildUrl("categories", {}));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        categoriesFinished(reply, mCachedCategories);
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

void Giphy::searchGifs(const QString& query, int offset)
{
    if (mSearchInProgress)
    {
        qWarning() << "Search already in progress";
        return;
    }

    if (offset == 0)
    {
        mNextOffset.reset();
        mOverviewModel.clear();
        mQuery = query;
    }
    else
    {
        Q_ASSERT(query == mQuery);
    }

    // TODO: rating, field filtering
    Params params{{"q", query},
                  {"offset", QString::number(offset)}};

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

void Giphy::getNextPage()
{
    if (!mNextOffset)
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

    searchGifs(mQuery, *mNextOffset);
}

QUrl Giphy::buildUrl(const QString& endpoint, const Params& params) const
{
    QUrl url(GIPHY_BASE_URL + endpoint);
    QUrlQuery query;
    query.addQueryItem("api_key", QUrl::toPercentEncoding(mApiKey));

    if (!mLanguageCode.isEmpty())
        query.addQueryItem("lang", QUrl::toPercentEncoding(mLanguageCode));

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
}

void Giphy::searchGifsFinished(QNetworkReply* reply, const QString& query)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Search failed:" << reply->request().url() << "error:" <<
            reply->error() << reply->errorString();
        emit searchGifsFailed(reply->errorString());
        return;
    }

    TenorGifList tenorGifList;
    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    try {
        const auto paginationJson = xjson.getRequiredJsonObject("pagination");
        const ATProto::XJsonObject paginationXjson(paginationJson);
        const int resultCount = paginationXjson.getRequiredInt("count");
        const int offset = paginationXjson.getRequiredInt("offset");

        if (resultCount == 0)
        {
            qDebug() << "No more results";
            mNextOffset.reset();
            return;
        }

        const auto results = xjson.getRequiredVector<GiphyGif>("data");

        for (const auto& giphyGif : results)
        {
            TenorGif gif(giphyGif->getId(),
                         giphyGif->getDescription(),
                         query,
                         giphyGif->getOriginal()->getUrl(), giphyGif->getOriginal()->getSize(),
                         giphyGif->getFixedHeight()->getUrl(), giphyGif->getFixedHeight()->getSize(),
                         giphyGif->getOriginalStill()->getUrl(), giphyGif->getOriginalStill()->getSize());
            tenorGifList.append(gif);
        }

        mNextOffset = offset + resultCount;
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
    }

    mOverviewModel.addGifs(tenorGifList);
}

bool Giphy::categoriesFinished(QNetworkReply* reply, TenorCategoryList& categoryList)
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

    const auto categories = xjson.getOptionalArray("data");

    if (!categories)
    {
        qWarning("Data missing");
        return false;
    }

    for (const auto& category : *categories)
    {
        if (!category.isObject())
        {
            qWarning() << "Non-object element in data";
            continue;
        }

        const auto catJson = category.toObject();
        const auto catXJson = ATProto::XJsonObject(catJson);

        try {
            const auto name = catXJson.getRequiredString("name");
            const auto gif = catXJson.getRequiredObject<GiphyGif>("gif");
            const auto& gifUrl = gif->getFixedHeight()->getUrl();
            qDebug() << "Category:" << name << gifUrl;
            const TenorCategory cat(gifUrl, name);
            categoryList.append(cat);
        }
        catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid JSON:" << e.msg();
            continue;
        }
    }

    return true;
}

void Giphy::allCategoriesRetrieved()
{
    TenorCategoryList categoryList;

    if (mRecentCategory)
        categoryList.push_back(*mRecentCategory);

    for (const auto& category : mCachedCategories)
        categoryList.push_back(category);

    emit categories(categoryList);
}

}
