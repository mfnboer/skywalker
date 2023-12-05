// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "tenor_category.h"
#include "tenor_gif.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQmlIntegration>

namespace Skywalker {

class Tenor : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit Tenor(QObject* parent = nullptr);

    Q_INVOKABLE void searchGifs(const QString& query, const QString& pos = "");
    Q_INVOKABLE void getCategories();
    // TODO Q_INVOKABLE void registerShare(const QString& id, const QString& query);

signals:
    void searchGifsResult(const TenorGifList& results, const QString& next);
    void searchGifsFailed();
    void categories(const TenorCategoryList categories);

private:
    using Params = QList<QPair<QString, QString>>;
    QUrl buildUrl(const QString& endpoint, const Params& params) const;

    void getCategories(const QString& type, TenorCategoryList& categoryList, const std::function<void()>& getNext = {});
    void searchGifsFinished(QNetworkReply* reply);
    bool categoriesFinished(QNetworkReply* reply, TenorCategoryList& categoryList);
    void allCategoriesRetrieved();

    struct MediaFormat
    {
        QString mUrl;
        QSize mSize;
    };

    MediaFormat mediaFormatFromJson(const QJsonObject& json) const;

    const QString mApiKey;
    const QString mClientKey;
    QString mLocale;
    QNetworkAccessManager mNetwork;
    TenorCategoryList mCachedFeaturedCategories;
    TenorCategoryList mCachedTrendingCategories;
};

}
