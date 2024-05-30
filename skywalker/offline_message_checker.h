// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "bookmarks.h"
#include "content_filter.h"
#include "image_reader.h"
#include "muted_words.h"
#include "notification_list_model.h"
#include "user_settings.h"
#include <atproto/lib/client.h>

#if defined(Q_OS_ANDROID)
extern "C" {
// Will be called from the Android WorkManager through System.loadLibrary
// QT JNI registrations cannot be used. Those are destroyed when the app exits.
JNIEXPORT int JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv*, jobject, jstring jSettingsFileName, jstring jLibDir);
}
#endif

namespace Skywalker {

struct NotificationChannel
{
    QString mId;
    QString mName;
    QString mDescription;
};

// Checks if there are new messages and raises app notifications.
// This is run from the Android WorkManager in a background thread when the app is not running!
class OffLineMessageChecker
{
public:
    static const std::vector<NotificationChannel> NOTIFCATION_CHANNELS;

    // Start background process that periodically checks for new messages.
    static void start(bool wifiOnly);

    static void createNotificationChannels();
    static bool checkNotificationPermission();

    explicit OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp);
    explicit OffLineMessageChecker(const QString& settingsFileName, QEventLoop* eventLoop);

    int check();

private:
    // Values must be equal to the values in NewMessageNotifier.java
    enum class IconType
    {
        POST = 0,
        FOLLOW = 1,
        LIKE = 2,
        MENTION = 3,
        REPOST = 4,
        CHAT = 5
    };

    int startEventLoop();
    void exit(int exitCode);
    void resumeSession(bool retry = false);
    bool getSession(QString& host, ATProto::ComATProtoServer::Session& session);
    void saveSession(const ATProto::ComATProtoServer::Session& session);
    void refreshSession();
    void getUserPreferences();
    void checkUnreadNotificationCount();
    void getNotifications(int toRead);
    void getChatNotifications();
    void getAvatars();
    void getAvatars(const QStringList& urls);
    void createNotifications();
    void createNotification(const Notification& notification);
    void createNotification(const QString channelId, const BasicProfile& author, const QString& msg, const QDateTime& when, IconType iconType);

    QCoreApplication* mBackgroundApp = nullptr;
    QEventLoop* mEventLoop = nullptr;
    UserSettings mUserSettings;
    std::unique_ptr<ATProto::Client> mBsky;
    QString mUserDid;
    ImageReader mImageReader;
    ATProto::UserPreferences mUserPreferences;
    ContentFilter mContentFilter;
    Bookmarks mBookmarks; // Not loaded. Needed for notificaion model
    MutedWords mMutedWords;
    NotificationListModel mNotificationListModel;
    std::unordered_map<QString, QByteArray> mAvatars; // URL -> jpg
};

}
