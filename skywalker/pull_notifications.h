// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"

#if defined(Q_OS_ANDROID)
extern "C" {
JNIEXPORT void JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv*, jobject);
}
#endif

namespace Skywalker {

class PullNotifications
{
public:
    PullNotifications(bool createChannel = true);

    void startNewMessageChecker();
    static void checkNewMessages();

private:
    bool isEnabled() const;
    bool checkPermission();
    void createNotificationChannel();
    static void createNotification(const QString& title, const QString& msg);

    UserSettings mUserSettings;
};

}
