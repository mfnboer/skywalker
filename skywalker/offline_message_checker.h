// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include "image_reader.h"
#include "list_store.h"
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
class OffLineMessageChecker : public QObject
{
public:
    using StoppedCb = std::function<void()>;

    static const std::vector<NotificationChannel> NOTIFCATION_CHANNELS;

    // Start background process that periodically checks for new messages.
    // checkNotificationPermission must have been called before.
    static void start(bool wifiOnly);

    // The background process should be stopped from SkywalkerActivity (Java) when the
    // app starts/resumes. However if the app pause and resumes within 120ms, then the Android
    // (Java) layer does not seem to have paused!!
    static void forceStop();
    static void waitForStop(const StoppedCb& stoppedCb);

    static void init(UserSettings* userSettings);
    static void checkNotificationPermission();

    explicit OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp);
    explicit OffLineMessageChecker(const QString& settingsFileName, QEventLoop* eventLoop);

#if defined(Q_OS_ANDROID)
    int check(jobject newMessageChecker);
#endif

private:
    // Values must be equal to the values in NewMessageNotifier.java
    enum class IconType
    {
        POST = 0,
        FOLLOW = 1,
        LIKE = 2,
        MENTION = 3,
        REPOST = 4,
        CHAT = 5,
        VERIFICATION = 6,
    };

    static void createNotificationChannels();
    static void stopWaiting();

    void initNetwork();
    void reset();
    int startEventLoop();
    void exit(int exitCode);
#if defined(Q_OS_ANDROID)
    int check(jobject newMessageChecker, const QString& did);
    bool isStopped(jobject newMessageChecker);
#endif
    void resumeSession(const QString& did, bool retry = false);
    void saveSession(const ATProto::ComATProtoServer::Session& session);
    void refreshSession();
    void getUserPreferences();
    void getNotificationPreferences();
    void checkUnreadNotificationCount();
    void getNotifications(int toRead);
    void filterNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr) const;
    void getChatNotifications();
    void getAvatars();
    void getAvatars(const QStringList& urls);
    void createNotifications();
    void createNotification(const Notification& notification);
    void createNotification(const QString channelId, const BasicProfile& author, const QString& msg, const QDateTime& when, IconType iconType);
    QString getNotificationText(const Post& post) const;
    QString getNotificationText(const PostRecord& postRecord) const;

    static bool sNotificationPermissionGranted;
    static UserSettings* sUserSettings;
    static StoppedCb sStoppedCb;
    static QTimer sWaitForStopTimer;

    QNetworkAccessManager* mNetwork;
    QCoreApplication* mBackgroundApp = nullptr;
    QEventLoop* mEventLoop = nullptr;
    UserSettings mUserSettings;
    ATProto::Client::SharedPtr mBsky;
    QString mUserDid;
    ImageReader mImageReader;
    ATProto::UserPreferences mUserPreferences;
    ATProto::AppBskyNotification::Preferences::SharedPtr mNotificationPrefs;
    ListStore mContentFilterPolicies;
    ContentFilter mContentFilter;
    MutedWords mMutedWords;
    NotificationListModel mNotificationListModel;
    std::unordered_map<QString, QByteArray> mAvatars; // URL -> jpg/png
    QObject mPresence;
};

}
