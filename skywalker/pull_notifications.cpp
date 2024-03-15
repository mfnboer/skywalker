// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "pull_notifications.h"

#ifdef Q_OS_ANDROID
#include "android_utils.h"
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif



namespace Skywalker {

PullNotifications::PullNotifications(UserSettings& userSettings) :
    mUserSettings(userSettings)
{
    createNotificationChannel();
}

void PullNotifications::startNewMessageChecker()
{
#if defined(Q_OS_ANDROID)
    if (!isEnabled())
        return;

    if (!checkPermission())
        return;

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/NewMessageChecker", "startChecker");
#endif
}

// TODO: remove? notifications can be enabled/disabled via Android permissions
bool PullNotifications::isEnabled() const
{
#if defined(Q_OS_ANDROID)
    const QString did = mUserSettings.getActiveUserDid();

    if (!mUserSettings.getPullNotifications(did))
    {
        qDebug() << "Pull notifications not enabled";
        return true; // TODO
    }

    return true;
#else
    return false;
#endif
}

bool PullNotifications::checkPermission()
{
#if defined(Q_OS_ANDROID)
    static constexpr char const* POST_NOTIFICATIONS = "android.permission.POST_NOTIFICATIONS";
    const QString did = mUserSettings.getActiveUserDid();

    if (!AndroidUtils::checkPermission(POST_NOTIFICATIONS))
    {
        qDebug() << "No permission:" << POST_NOTIFICATIONS;
        mUserSettings.setPullNotifications(did, false);
        return false;
    }

    return true;
#else
    return false;
#endif
}

void PullNotifications::createNotificationChannel()
{
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/NewMessageNotifier", "createNotificationChannel");
#endif
}

}
