// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "web_service_base.h"
#include <QUrlQuery>

namespace Skywalker {

WebServiceBase::WebServiceBase(const QString& baseUrl, QNetworkAccessManager* network) :
    mBaseUrl(baseUrl),
    mNetwork(network)
{
}

QUrl WebServiceBase::buildUrl(const QString& endpoint, const Params& params) const
{
    QUrl url(mBaseUrl + endpoint);
    QUrlQuery query;

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
}

QNetworkReply* WebServiceBase::sendRequest(const QString& endpoint, const Params& params,
                                 const FinishedCb& finishedCb,
                                 const ErrorOccurredCb& errorOccurredCb,
                                 const SslErrorsCb& sslErrorsCb)
{
    QNetworkRequest request(buildUrl(endpoint, params));
    QNetworkReply* reply = mNetwork->get(request);

    if (finishedCb)
    {
        QObject::connect(reply, &QNetworkReply::finished,
            [presence=getPresence(), reply, finishedCb]{
                if (!presence)
                    return;

                finishedCb(reply);
            });
    }

    if (errorOccurredCb)
    {
        QObject::connect(reply, &QNetworkReply::errorOccurred,
            [presence=getPresence(), reply, errorOccurredCb](QNetworkReply::NetworkError errCode){
                if (!presence)
                    return;

                errorOccurredCb(reply, errCode);
            });
    }

    if (sslErrorsCb)
    {
        QObject::connect(reply, &QNetworkReply::sslErrors,
            [presence=getPresence(), reply, sslErrorsCb]{
                if (!presence)
                    return;

                sslErrorsCb(reply);
            });
    }

    return reply;
}

}
