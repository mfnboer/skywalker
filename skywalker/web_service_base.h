// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace Skywalker {

class WebServiceBase : public Presence
{
public:
    WebServiceBase(const QString& baseUrl, QNetworkAccessManager* network);
    virtual ~WebServiceBase() = default;

protected:
    using Params = QList<QPair<QString, QString>>;
    using FinishedCb = std::function<void(QNetworkReply*)>;
    using ErrorOccurredCb = std::function<void(QNetworkReply*, QNetworkReply::NetworkError)>;
    using SslErrorsCb = std::function<void(QNetworkReply*)>;

    static void addOptionalIntParam(Params& params, const QString& name, std::optional<int> value);
    static void addStringListParam(Params& params, const QString& name, const std::vector<QString>& list);

    void setBaseUrl(const QString& baseUrl) { mBaseUrl = baseUrl; }
    void setUserAgent(const QString& userAgent) { mUserAgent = userAgent; }
    virtual QUrl buildUrl(const QString& endpoint, const Params& params) const;
    QNetworkReply* sendRequest(const QString& endpoint, const Params& params,
                     const FinishedCb& finishedCb,
                     const ErrorOccurredCb& errorOccurredCb = {},
                     const SslErrorsCb& sslErrorsCb = {});

    QString mBaseUrl;
    QNetworkAccessManager* mNetwork = nullptr;
    QString mUserAgent;
};

}
