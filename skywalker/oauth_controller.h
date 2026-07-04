// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/oauth.h>
#include <QUrl>

#ifndef Q_OS_ANDROID
#include <QHttpServer>
#endif

namespace Skywalker {

class OAuthController
{
public:
    using RedirectCb = std::function<void(QUrl url)>;

    static constexpr int LISTEN_PORT = 1970;
#ifdef Q_OS_ANDROID
    static constexpr char const* REDIRECT_URL = "eu.thereforeiam:/skywalker/oauth/callback";
#else
    static constexpr char const* REDIRECT_URL = "http://127.0.0.1:1970/oauth/callback";
#endif

    static QString getClientId();
    static QStringList getScope();

    // When you add new scopes for a new release, add one of them here
    // to check and give an error message when it is not there, telling the user
    // to re-login.
    static std::optional<ATProto::OAuth::ScopeCheck> getScopeToCheckOnResume();

#ifndef Q_OS_ANDROID
    static QString getKeyStorageFilename(const QString& did);
    static QString getTestPassPhrase() ;
#endif

    bool start(const RedirectCb& redirectCb);
    void redirect(const QString& url);

private:
#ifdef DEBUG
    static QString getTestClientId();
    static QStringList getTestScope();
#endif

    RedirectCb mRedirectCb;
#ifndef Q_OS_ANDROID
    QHttpServer mHttpServer;
#endif
};

}
