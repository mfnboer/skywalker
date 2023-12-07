// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "link_card.h"
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

    Q_INVOKABLE void getLinkCard(const QString& link);
    Q_INVOKABLE LinkCard* makeLinkCard(const QString& link, const QString& title,
                                  const QString& description, const QString& thumb);

signals:
    void linkCard(LinkCard*);

private:
    void extractLinkCard(QNetworkReply* reply);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);

    QNetworkAccessManager mNetwork;
    QCache<QUrl, LinkCard> mCardCache;
    QNetworkReply* mInProgress = nullptr;
};

}
