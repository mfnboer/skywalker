// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "web_service_base.h"
#include "skywalker.h"
#include <QUrlQuery>

namespace Skywalker {

void WebServiceBase::addOptionalIntParam(Params& params, const QString& name, std::optional<int> value)
{
    if (value)
        params.append({name, QString::number(*value)});
}

void WebServiceBase::addStringListParam(Params& params, const QString& name, const std::vector<QString>& list)
{
    for (const auto& str : list)
        params.append({name, str});
}

WebServiceBase::WebServiceBase(const QString& baseUrl, QNetworkAccessManager* network) :
    mBaseUrl(baseUrl),
    mNetwork(network),
    mUserAgent(Skywalker::getUserAgentString())
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

    if (!mUserAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);

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
