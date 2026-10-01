// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>

namespace Skywalker {

class Http1NetworkAccessManager : public QNetworkAccessManager
{
public:
    using QNetworkAccessManager::QNetworkAccessManager;

protected:
    QNetworkReply* createRequest(Operation op, const QNetworkRequest& req,
                                 QIODevice* outgoing = nullptr) override;
};

class Http1NetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    QNetworkAccessManager* create(QObject* parent) override;
};

}