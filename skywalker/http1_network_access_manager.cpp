// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "http1_network_access_manager.h"
#include "skywalker.h"

namespace Skywalker {

QNetworkReply* Http1NetworkAccessManager::createRequest(Operation op, const QNetworkRequest& req,
                                                        QIODevice* outgoing)
{
    QNetworkRequest request(req);

    // Some image servers don't support HTTP/2, e.g. www.itmagazine.ch
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    // Required for www.itmagazine.ch
    request.setRawHeader("User-Agent", Skywalker::getUserAgentString().toUtf8());

    return QNetworkAccessManager::createRequest(op, request, outgoing);
}

QNetworkAccessManager* Http1NetworkAccessManagerFactory::create(QObject* parent)
{
    return new Http1NetworkAccessManager(parent);
}

}