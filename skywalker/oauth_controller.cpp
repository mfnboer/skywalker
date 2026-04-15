// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "oauth_controller.h"
#ifdef Q_OS_ANDROID
#include "jni_callback.h"
#else
#include "file_utils.h"
#endif
#include <atproto/lib/oauth.h>
#include <QTcpServer>

namespace Skywalker {

const QStringList OAuthController::SCOPE = {
    ATProto::OAuth::SCOPE_ATPROTO,
    ATProto::OAuth::SCOPE_TRANSITION_GENERIC,
    ATProto::OAuth::SCOPE_TRANSITION_CHAT,
    ATProto::OAuth::SCOPE_TRANSITION_EMAIL
};

#ifndef Q_OS_ANDROID
QString OAuthController::getKeyStorageFilename(const QString& did)
{
    return QString("%1/dpop.pem").arg(FileUtils::getAppDataPath(did));
}

QString OAuthController::getTestPassPhrase()
{
    return "Rage! Rage! Against the dying of the light.";
}
#endif

bool OAuthController::start(const RedirectCb& redirectCb)
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
#else
    // Intent handling may not have started yet. Make sure it is.
    JNICallbackListener::handlePendingIntent();
    mRedirectCb = redirectCb;
    return true;
#endif
}

#ifdef Q_OS_ANDROID
void OAuthController::redirect(const QString& url)
{
    qDebug() << "Redirect:" << url;

    if (!mRedirectCb)
    {
        qWarning() << "Redirect call back not set";
        return;
    }

    mRedirectCb(url);
}
#endif

}
