// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#ifndef Q_OS_ANDROID
#include <QHttpServer>
#endif

namespace Skywalker {

// TODO: rename
class OAuthRedirect
{
public:
    using RedirectCb = std::function<void(QUrl url)>;

    static constexpr int LISTEN_PORT = 1970;
    static constexpr char const* CLIENT_ID = "https://mfnboer.home.xs4all.nl/skywalker/oauth/client-metadata.json";
    static constexpr char const* REDIRECT_URL = "http://127.0.0.1:1970/oauth/callback";
    static const QStringList SCOPE;

#ifndef Q_OS_ANDROID
    static QString getKeyStorageFilename(const QString& did);
    static QString getTestPassPhrase() ;
#endif

    bool start(const RedirectCb& redirectCb);

private:
#ifndef Q_OS_ANDROID
    QHttpServer mHttpServer;
#endif
};

}
