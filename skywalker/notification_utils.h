// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "edit_notification_preferences.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class NotificationUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotificationUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getNotificationPrefs();

signals:
    void notificationPrefsOk(EditNotificationPreferences*);
    void notificationPrefsFailed(QString error);

private:
    std::unique_ptr<EditNotificationPreferences> mNotificationPrefs;
};

}
