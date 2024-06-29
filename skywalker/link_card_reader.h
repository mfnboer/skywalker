// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "link_card.h"
#include "gif_utils.h"
#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQmlIntegration>

namespace Skywalker {

class LinkCardReader : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LinkCardReader(QObject* parent = nullptr);

    Q_INVOKABLE void getLinkCard(const QString& link, bool retry = false);
    Q_INVOKABLE LinkCard* makeLinkCard(const QString& link, const QString& title,
                                  const QString& description, const QString& thumb);

signals:
    void linkCard(LinkCard*);
    void linkCardFailed();

private:
    QString toPlainText(const QString& text);
    void extractLinkCard(QNetworkReply* reply);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);
    void redirect(QNetworkReply* reply, const QUrl& redirectUrl);

    QNetworkAccessManager mNetwork;
    QCache<QUrl, LinkCard> mCardCache;
    QNetworkReply* mInProgress = nullptr;
    QUrl mPrevDestination;
    bool mRetry = false;
    GifUtils mGifUtils;
};

}
