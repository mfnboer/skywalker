// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "edit_notification_preferences.h"
#include "presence.h"
#include "profile.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/notification_master.h>

namespace Skywalker {

class NotificationUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit NotificationUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getNotificationPrefs();
    Q_INVOKABLE void saveNotificationPrefs();
    Q_INVOKABLE void subscribeActivity(const QString& did, bool posts, bool replies);

signals:
    void notificationPrefsOk(EditNotificationPreferences*);
    void notificationPrefsFailed(QString error);
    void subscribeActivityOk(QString did, ActivitySubscription subscription);
    void subscribeActivityFailed(QString error);

private:
    ATProto::NotificationMaster* notificationMaster();
    void getNotificationDeclaration();
    void updateNotificationDeclaration(QEnums::AllowActivitySubscriptionsType allowSubscriptions);
    bool arePreferencesModified() const;

    ATProto::AppBskyNotification::Preferences::SharedPtr mNotificationPrefs;
    std::unique_ptr<ATProto::NotificationMaster> mNotificationMaster;
    std::unique_ptr<EditNotificationPreferences> mEditNotificationPrefs;
};

}
