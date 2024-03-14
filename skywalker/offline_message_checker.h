// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include <atproto/lib/client.h>

#if defined(Q_OS_ANDROID)
extern "C" {
// Will be called from the Android WorkManager through System.loadLibrary
// QT JNI registrations cannot be used. Those are destroyed when the app exits.
JNIEXPORT void JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv*, jobject, jstring jSettingsFileName, jstring jLibDir);
}
#endif

namespace Skywalker {

// Checks if there are new messages and raises app notifications.
// This is run from the Android WorkManager in a background thread when the app is not running!
class OffLineMessageChecker
{
public:
    explicit OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp);
    explicit OffLineMessageChecker(const QString& settingsFileName, QEventLoop* eventLoop);

    void run();

private:
    static void createNotification(const QString& title, const QString& msg);

    void startEventLoop();
    void exit();
    void resumeSession();
    void checkNewMessages();
    bool getSession(QString& host, ATProto::ComATProtoServer::Session& session);
    void saveSession(const ATProto::ComATProtoServer::Session& session);
    void refreshSession();
    void login();

    QCoreApplication* mBackgroundApp = nullptr;
    QEventLoop* mEventLoop = nullptr;
    UserSettings mUserSettings;
    std::unique_ptr<ATProto::Client> mBsky;
    QString mUserDid;
};

}
