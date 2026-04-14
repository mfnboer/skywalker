// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "oauth_redirect.h"
#include <atproto/lib/oauth.h>
#include <QTcpServer>

namespace Skywalker {

const QStringList OAuthRedirect::SCOPE = {
    ATProto::OAuth::SCOPE_ATPROTO,
    ATProto::OAuth::SCOPE_TRANSITION_GENERIC,
    ATProto::OAuth::SCOPE_TRANSITION_CHAT,
    ATProto::OAuth::SCOPE_TRANSITION_EMAIL
};

bool OAuthRedirect::start(const RedirectCb& redirectCb)
{
#ifndef Q_OS_ANDROID
    qDebug() << "Start HTTP server";
    auto* tcpServer = new QTcpServer();

    mHttpServer.route("/oauth/callback",
        [redirectCb](const QHttpServerRequest& request, QHttpServerResponder& responder){
            qDebug() << "oauth callback:" << request.url();
            redirectCb(request.url());
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        });

    if (!tcpServer->listen(QHostAddress::LocalHost, LISTEN_PORT) || !mHttpServer.bind(tcpServer))
    {
        qWarning() << "Failed to listen on port:" << LISTEN_PORT;
        delete tcpServer;
        return false;
    }

    qDebug() << "Listening on port:" << tcpServer->serverPort();
    return true;
#endif
}

}
