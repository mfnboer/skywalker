// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "songlink_links.h"
#include <QNetworkAccessManager>
#include <QObject>

namespace Skywalker {

class Songlink : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool inProgress READ isInProgress NOTIFY inProgressChanged FINAL)
    QML_ELEMENT

public:
    explicit Songlink(QObject* parent = nullptr);

    Q_INVOKABLE static bool isMusicLink(const QString& link);
    Q_INVOKABLE static bool isCached(const QString& link);
    Q_INVOKABLE void getLinks(const QString& musicLink);
    bool isInProgress() const { return mInProgress; }

signals:
    void linksFound(SonglinkLinks);
    void failure(QString error);
    void inProgressChanged();

private:
    void init();
    QUrl buildUrl(const QString& musicLink) const;
    void setInProgress(bool inProgress);
    void processGetLinksReply(QNetworkReply* reply, const QString& musicLink);

    bool mInProgress = false;
    QString mCountryCode;
    QNetworkAccessManager* mNetwork = nullptr;
};

}
