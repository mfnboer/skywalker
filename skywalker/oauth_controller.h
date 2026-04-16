// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#ifndef Q_OS_ANDROID
#include <QHttpServer>
#endif

namespace Skywalker {

// TODO: move to skywalker.thereforeiam.eu
class OAuthController
{
public:
    using RedirectCb = std::function<void(QUrl url)>;

    static constexpr int LISTEN_PORT = 1970;
    static constexpr char const* CLIENT_ID = "https://mfnboer.home.xs4all.nl/skywalker/oauth/client-metadata.json";
#ifdef Q_OS_ANDROID
    static constexpr char const* REDIRECT_URL = "nl.xs4all.home.mfnboer:/oauth/callback";
#else
    static constexpr char const* REDIRECT_URL = "http://127.0.0.1:1970/oauth/callback";
#endif
    static const QStringList SCOPE;

#ifndef Q_OS_ANDROID
    static QString getKeyStorageFilename(const QString& did);
    static QString getTestPassPhrase() ;
#endif

    bool start(const RedirectCb& redirectCb);
    void redirect(const QString& url);

private:
    RedirectCb mRedirectCb;
#ifndef Q_OS_ANDROID
    QHttpServer mHttpServer;
#endif
};

}
