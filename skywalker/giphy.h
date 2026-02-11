// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "tenor_category.h"
#include "tenor_gif_overview_model.h"
#include "wrapped_skywalker.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Skywalker {

class Giphy : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(TenorOverviewModel* overviewModel READ getOverviewModel CONSTANT FINAL)
    Q_PROPERTY(int width READ getWidth WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(int spacing READ getSpacing WRITE setSpacing NOTIFY spacingChanged FINAL)
    Q_PROPERTY(bool searchInProgress READ getSearchInProgress WRITE setSearchInProgress NOTIFY searchInProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit Giphy(QObject* parent = nullptr);

    Q_INVOKABLE void searchGifs(const QString& query, int offset = 0);
    Q_INVOKABLE void getNextPage();
    Q_INVOKABLE void getCategories();

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

private:
    using Params = QList<QPair<QString, QString>>;

    QUrl buildUrl(const QString& endpoint, const Params& params) const;
    void searchGifsFinished(QNetworkReply* reply, const QString& query);
    bool categoriesFinished(QNetworkReply* reply, TenorCategoryList& categoryList);
    void allCategoriesRetrieved();

    const QString mApiKey;
    QString mLanguageCode;
    TenorCategoryList mCachedCategories;
    std::optional<TenorCategory> mRecentCategory;
    int mWidth = 300;
    int mSpacing = 4;
    TenorOverviewModel mOverviewModel;
    QString mQuery;
    std::optional<int> mNextOffset;
    bool mSearchInProgress = false;
    QNetworkAccessManager* mNetwork;
};

}
