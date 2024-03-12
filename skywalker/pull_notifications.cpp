// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "pull_notifications.h"
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include "android_utils.h"
#include "jni_utils.h"
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif

#if defined(Q_OS_ANDROID)
JNIEXPORT void JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv*, jobject)
{
    qDebug() << "CHECK NEW MESSAGES 1";
    Skywalker::PullNotifications pullNotifications{false};
    qDebug() << "CHECK NEW MESSAGES 2";
    pullNotifications.checkNewMessages();
    qDebug() << "CHECK NEW MESSAGES 3";
}
#endif

namespace Skywalker {

PullNotifications::PullNotifications(bool createChannel)
{
    if (createChannel)
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

void PullNotifications::checkNewMessages()
{
#if defined(Q_OS_ANDROID)
    //const QString did = mUserSettings.getActiveUserDid();
    //qDebug() << "Check messages for:" << did;
    qDebug() << "DUMMY MESSAGE";
    createNotification("Michel Bestaat", "Test post");
#endif
}

bool PullNotifications::isEnabled() const
{
#if defined(Q_OS_ANDROID)
    const QString did = mUserSettings.getActiveUserDid();

    if (!mUserSettings.getPullNotifications(did))
    {
        qDebug() << "Pull notifications not enabled";
        return false; // TODO
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

void PullNotifications::createNotification(const QString& title, const QString& msg)
{
#if defined(Q_OS_ANDROID)
    QJniEnvironment env;
    QJniObject jTitle = QJniObject::fromString(title);
    QJniObject jMsg = QJniObject::fromString(msg);

    auto [javaClass, methodId] = JniUtils::getClassAndMethod(
        env,
        "com/gmail/mfnboer/NewMessageNotifier",
        "createNotification",
        "(Ljava/lang/String;Ljava/lang/String;)V");

    if (!javaClass || !methodId)
        return;

    QJniObject::callStaticMethod<void>(
        javaClass,
        methodId,
        jTitle.object<jstring>(),
        jMsg.object<jstring>());
#else
    Q_UNUSED(title);
    Q_UNUSED(msg);
#endif
}

}
