// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "notification_utils.h"

namespace Skywalker {

NotificationUtils::NotificationUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void NotificationUtils::getNotificationPrefs()
{
    if (!bskyClient())
        return;

    bskyClient()->getNotificationPreferences(
        [this, presence=getPresence()](auto prefs){
            if (!presence)
                return;

            mNotificationPrefs = std::make_unique<EditNotificationPreferences>(prefs->mPreferences);
            emit notificationPrefsOk(mNotificationPrefs.get());
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getNotificationPreferences failed:" << error << " - " << msg;
            emit notificationPrefsFailed(msg);
        });
}

}
