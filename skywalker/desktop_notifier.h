// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QString>
#include <atproto/lib/client.h>

namespace Skywalker {

// Shows desktop notifications through the freedesktop notification service.
// On platforms without that service this does nothing.
class DesktopNotifier : public QObject
{
    Q_OBJECT

public:
    explicit DesktopNotifier(QObject* parent = nullptr);

    void notify(const QString& title, const QString& body);

    // Fetch the newest notifications and show the ones that are enabled for push
    // notifications in the account's notification preferences.
    void notifyNew(ATProto::Client* bsky, int count);

private:
    void showNew(ATProto::Client* bsky, ATProto::AppBskyNotification::Preferences::SharedPtr prefs, int count);
};

}
