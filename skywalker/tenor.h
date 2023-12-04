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
    explicit Tenor(const QString& apiKey, const QString& clientKey, QObject* parent = nullptr);

    //Q_INVOKABLE void searchGifs(const QString& query, const QString& pos = "");
    //Q_INVOKABLE void getCategories();
    //Q_INVOKABLE void registerShare(const QString& id, const QString& query);

signals:
    void searchGifsResult(const TenorGifList& results, const QString& next);
    void searchGifsFailed();
    void categories(const TenorCategoryList& categories);
    void categoriesFailed();

private:
    const QString mApiKey;
    const QString mClientKey;
    QNetworkAccessManager mNetwork;
    TenorCategoryList mCachedCategories;
};

}
