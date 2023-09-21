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

signals:
    void linkCard(LinkCard*);

private:
    QString toPlainText(const QString& text) const;
    void extractLinkCard(QNetworkReply* reply);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);

    QNetworkAccessManager mNetwork;
    QCache<QUrl, LinkCard> mCardCache;
    QNetworkReply* mInProgress = nullptr;
};

}
