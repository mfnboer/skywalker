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

    // Fetch the newest notifications and show their content. Falls back to a
    // notification with only the count if the notifications cannot be retrieved.
    void notifyNew(ATProto::Client* bsky, const QString& accountHandle, int count);
};

}
