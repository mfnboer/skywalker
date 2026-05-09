// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "tenor_category.h"
#include "tenor_gif_overview_model.h"
#include "web_service_base.h"
#include "wrapped_skywalker.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQmlIntegration>

namespace Skywalker {

class Tenor : public WrappedSkywalker, public WebServiceBase
{
    Q_OBJECT
    Q_PROPERTY(TenorOverviewModel* overviewModel READ getOverviewModel CONSTANT FINAL)
    Q_PROPERTY(int width READ getWidth WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(int spacing READ getSpacing WRITE setSpacing NOTIFY spacingChanged FINAL)
    Q_PROPERTY(bool searchInProgress READ getSearchInProgress WRITE setSearchInProgress NOTIFY searchInProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit Tenor(QObject* parent = nullptr);
    virtual ~Tenor() = default;

    Q_INVOKABLE void searchRecentGifs();
    Q_INVOKABLE void searchGifs(const QString& query, const QString& pos = "");
    Q_INVOKABLE void getNextPage();
    Q_INVOKABLE void getCategories();
    Q_INVOKABLE void registerShare(const TenorGif& gif);
    Q_INVOKABLE void addRecentGif(const TenorGif& gif);

    int getWidth() const { return mWidth; }
    void setWidth(int width);
    int getSpacing() const { return mSpacing; }
    void setSpacing(int spacing);
    bool getSearchInProgress() const { return mSearchInProgress; }
    void setSearchInProgress(bool inProgress);
    TenorOverviewModel* getOverviewModel() { return &mOverviewModel; }

signals:
    void searchGifsFailed(const QString& error);
    void categories(const TenorCategoryList categories);
    void widthChanged();
    void spacingChanged();
    void searchInProgressChanged();

protected:
    void setApiKey(const QString& apiKey) { mApiKey = apiKey; }
    void setClientKey(const QString& clientKey) { mClientKey = clientKey; }
    virtual TenorGif toTenorGif(const QJsonValue& resultElem, const QString& query) const;
    virtual void setRecentGifs(const QString& did, const QStringList& gifIds);
    virtual QStringList getRecentGifs(const QString& did) const;

private:
    using Params = QList<QPair<QString, QString>>;
    QUrl buildUrl(const QString& endpoint, const Params& params) const override;

    void getCategories(const QString& type, TenorCategoryList& categoryList, const std::function<void()>& getNext = {});
    void getRecentCategory();
    void setRecentCategory(QNetworkReply* reply);
    void setRecentCategory(const TenorGif& gif);
    void searchGifsFinished(QNetworkReply* reply, const QString& query);
    bool categoriesFinished(QNetworkReply* reply, TenorCategoryList& categoryList);
    void allCategoriesRetrieved();
    QStringList getRecentGifs();

    struct MediaFormat
    {
        QString mUrl;
        QSize mSize;
    };

    MediaFormat mediaFormatFromJson(const QJsonObject& json) const;
    QString mediaUrlFromJson(const QJsonObject& json) const;

    QString mApiKey;
    QString mClientKey;
    QString mLocale;
    TenorCategoryList mCachedFeaturedCategories;
    TenorCategoryList mCachedTrendingCategories;
    std::optional<TenorCategory> mRecentCategory;
    int mWidth = 300;
    int mSpacing = 4;
    TenorOverviewModel mOverviewModel;
    QString mQuery;
    QString mNextPos;
    bool mSearchInProgress = false;
};

class Klipy : public Tenor
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Klipy(QObject* parent = nullptr);

protected:
    virtual TenorGif toTenorGif(const QJsonValue& resultElem, const QString& query) const override;
    virtual void setRecentGifs(const QString& did, const QStringList& gifIds) override;
    virtual QStringList getRecentGifs(const QString& did) const override;
};

}
