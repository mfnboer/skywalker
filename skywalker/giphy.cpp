// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "giphy.h"
#include "giphy_gif.h"
#include "skywalker.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

namespace {

constexpr char const* GIPHY_BASE_URL = "https://api.giphy.com/v1/";
constexpr char const* FIELDS_FILTER = "id,title,alt_text,id,images.original,images.fixed_height,images.original_still";
constexpr char const* RATING = "pg";
constexpr int MAX_RECENT_GIFS = 50;

TenorGif toTenorGif(GiphyGif::SharedPtr giphyGif, const QString& query)
{
    TenorGif gif(giphyGif->getId(),
                 giphyGif->getTitle(),
                 query,
                 giphyGif->getOriginal()->getUrl(), giphyGif->getOriginal()->getSize(),
                 giphyGif->getFixedHeight()->getUrl(), giphyGif->getFixedHeight()->getSize(),
                 giphyGif->getOriginalStill()->getUrl(), giphyGif->getOriginalStill()->getSize(),
                 giphyGif->getOriginal()->getMp4Url());
    gif.setIsGiphy(true);
    return gif;
}

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
    getRecentCategory();
    getTrendingCategory();

    if (!mCachedCategories.empty())
        return;

    QNetworkRequest request(buildUrl("gifs/categories", {}));
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

void Giphy::addRecentGif(const TenorGif& gif)
{
    const QString did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList gifIds = settings->getRecentGiphyGifs(did);

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
    settings->setRecentGiphyGifs(did, gifIds);
    setRecentCategory(gif);
}

void Giphy::searchTrendingGifs(int offset)
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
        mQuery.clear();
        mTrendingSearch = true;
    }
    else
    {
        Q_ASSERT(mTrendingSearch);
    }

    Params params{{"offset", QString::number(offset)},
                  {"rating", RATING},
                  {"fields", FIELDS_FILTER}};

    QNetworkRequest request(buildUrl("gifs/trending", params));
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

void Giphy::searchRecentGifs()
{
    if (mSearchInProgress)
    {
        qWarning() << "Search already in progress";
        return;
    }

    mNextOffset.reset();
    mOverviewModel.clear();
    mQuery.clear();
    mTrendingSearch = false;

    const QStringList gifIdList = getRecentGifs();

    if (gifIdList.empty())
        return;

    Q_ASSERT(gifIdList.size() <= 100); // maximum allowed by Giphy
    Params params{{"ids", gifIdList.join(',')}};

    setSearchInProgress(true);
    QNetworkRequest request(buildUrl("gifs", params));
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
        mTrendingSearch = false;
    }
    else
    {
        Q_ASSERT(query == mQuery);
    }

    Params params{{"q", query},
                  {"offset", QString::number(offset)},
                  {"rating", RATING},
                  {"fields", FIELDS_FILTER}};

    setSearchInProgress(true);
    QNetworkRequest request(buildUrl("gifs/search", params));
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

    Q_ASSERT(!mQuery.isEmpty() || mTrendingSearch);

    if (mTrendingSearch)
        searchTrendingGifs(*mNextOffset);
    else if (!mQuery.isEmpty())
        searchGifs(mQuery, *mNextOffset);
    else
        qWarning() << "No previous query available";
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

void Giphy::getTrendingCategory()
{
    if (mTrendingCategory)
    {
        allCategoriesRetrieved();
        return;
    }

    Params params{{"limit", "1"},
                  {"rating", RATING},
                  {"fields", FIELDS_FILTER}};

    QNetworkRequest request(buildUrl("gifs/trending", params));
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, presence=getPresence(), reply]{
        if (!presence)
            return;

        setTrendingCategory(reply);
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

void Giphy::setTrendingCategory(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qWarning() << "Get trending category failed:" << reply->request().url() << "error:" <<
            reply->error() << reply->errorString();
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto jsonObject = json.object();
    const ATProto::XJsonObject xjson(jsonObject);

    try {
        const auto results = xjson.getOptionalVector<GiphyGif>("data");

        if (results.empty())
        {
            qWarning("Failed to get recent category");
            return;
        }

        const auto gif = toTenorGif(results.front(), "");
        setTrendingCategory(gif);
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
    }
}

void Giphy::setTrendingCategory(const TenorGif& gif)
{
    qDebug() << "Set recent category";
    mTrendingCategory = TenorCategory(gif.getSmallUrl(), tr("Trending"), false, true);
}

void Giphy::getRecentCategory()
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

    Params params{{"ids", gifIds.front()}};

    QNetworkRequest request(buildUrl("gifs", params));
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

void Giphy::setRecentCategory(QNetworkReply* reply)
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

    try {
        const auto results = xjson.getOptionalVector<GiphyGif>("data");

        if (results.empty())
        {
            qWarning("Failed to get recent category");
            return;
        }

        const auto gif = toTenorGif(results.front(), "");
        setRecentCategory(gif);
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Invalid JSON:" << e.msg();
    }
}

void Giphy::setRecentCategory(const TenorGif& gif)
{
    qDebug() << "Set recent category";
    mRecentCategory = TenorCategory(gif.getSmallUrl(), tr("Recently Used"), true);
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
        auto pagination = xjson.getRequiredObject<Pagination>("pagination");

        if (pagination->mCount == 0)
        {
            qDebug() << "No more results";
            mNextOffset.reset();
            return;
        }

        const auto results = xjson.getRequiredVector<GiphyGif>("data");

        for (const auto& giphyGif : results)
        {
            const auto gif = toTenorGif(giphyGif, query);
            tenorGifList.append(gif);
        }

        mNextOffset = pagination->mOffset + pagination->mCount;
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

    categoryList.clear();

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

    if (mTrendingCategory)
        categoryList.push_back(*mTrendingCategory);

    for (const auto& category : mCachedCategories)
        categoryList.push_back(category);

    qDebug() << "All categories:" << categoryList.size();
    emit categories(categoryList);
}

QStringList Giphy::getRecentGifs()
{
    const QString did = mSkywalker->getUserDid();
    auto* settings = mSkywalker->getUserSettings();
    QStringList gifIds = settings->getRecentGiphyGifs(did);
    return gifIds;
}

Giphy::Pagination::SharedPtr Giphy::Pagination::fromJson(const QJsonObject& json)
{
    auto pagination = std::make_shared<Pagination>();
    ATProto::XJsonObject xjson(json);
    pagination->mOffset = xjson.getRequiredInt("offset");
    pagination->mCount = xjson.getRequiredInt("count");
    return pagination;
}

}
