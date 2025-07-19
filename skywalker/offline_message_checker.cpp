// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "offline_message_checker.h"
#include "notification.h"
#include "photo_picker.h"
#include "skywalker.h"

#ifdef Q_OS_ANDROID
#include "android_utils.h"
#include "jni_utils.h"
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#include <QTimer>
#include <QHostInfo>
#endif

using namespace std::chrono_literals;

namespace {
constexpr char const* CHANNEL_POST = "CHANNEL_POST";
constexpr char const* CHANNEL_LIKE = "CHANNEL_LIKE";
constexpr char const* CHANNEL_REPOST = "CHANNEL_REPOST";
constexpr char const* CHANNEL_FOLLOW = "CHANNEL_FOLLOW";
constexpr char const* CHANNEL_CHAT = "CHANNEL_CHAT";
constexpr char const* CHANNEL_VERIFICATION = "CHANNEL_VERIFICATION";
constexpr char const* CHANNEL_ACTIVITY_SUBSCRIPTION = "CHANNEL_ACTIVIY_SUBSCRIPTION";

constexpr int EXIT_OK = 0;
constexpr int EXIT_RETRY = -1;
constexpr int EXIT_NETWORK_FAILURE = -2;
}

#if defined(Q_OS_ANDROID)
JNIEXPORT int JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv* env, jobject, jstring jSettingsFileName, jstring jLibDir)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");
    qDebug() << "CHECK NEW MESSAGES START";

    const char* settingsFileName = (*env).GetStringUTFChars(jSettingsFileName, nullptr);

    if (!settingsFileName)
    {
        qWarning() << "Settings file name missing";
        return EXIT_OK;
    }

    const char* libDir = (*env).GetStringUTFChars(jLibDir, nullptr);

    if (!libDir)
    {
        qWarning() << "Library directory missing";
        return EXIT_OK;
    }

    // This makes networking work???
    // Somehow Qt fails to find this class if we don't do this lookup.
    QJniEnvironment jniEnv;
    jclass javaClass = jniEnv.findClass("org/qtproject/qt/android/network/QtNetwork");

    if (!javaClass)
    {
        qWarning() << "Class loading failed";
        return EXIT_OK;
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<QEventLoop> eventLoop;
    std::unique_ptr<Skywalker::OffLineMessageChecker> checker;

    if (!QCoreApplication::instance())
    {
        qDebug() << "Start core app";
        QCoreApplication::addLibraryPath(libDir);
        int argc = 1;
        const char* argv[] = {Skywalker::Skywalker::APP_NAME, NULL};
        app = std::make_unique<QCoreApplication>(argc, (char**)argv);
        qDebug() << "LIBS:" << app->libraryPaths();

        // Pass explicit file name as Qt does not know the app data path when running in
        // a background task.
        checker = std::make_unique<Skywalker::OffLineMessageChecker>(settingsFileName, app.get());
    }
    else
    {
        qDebug() << "App still running";
        eventLoop = std::make_unique<QEventLoop>();
        checker = std::make_unique<Skywalker::OffLineMessageChecker>(settingsFileName, eventLoop.get());
    }

    int exitCode = checker->check();

    (*env).ReleaseStringUTFChars(jSettingsFileName, settingsFileName);
    (*env).ReleaseStringUTFChars(jLibDir, libDir);
    qDebug() << "CHECK NEW MESSAGES END:" << exitCode;

    return exitCode;
}
#endif

namespace Skywalker {

bool OffLineMessageChecker::sNotificationPermissionGranted = false;

const std::vector<NotificationChannel> OffLineMessageChecker::NOTIFCATION_CHANNELS = {
    {
        CHANNEL_POST,
        QObject::tr("Posts"),
        QObject::tr("Replies to your posts, mentions and quoted posts")
    },
    {
        CHANNEL_LIKE,
        QObject::tr("Likes"),
        QObject::tr("Likes on your posts")
    },
    {
        CHANNEL_REPOST,
        QObject::tr("Reposts"),
        QObject::tr("Reposts of your posts")
    },
    {
        CHANNEL_FOLLOW,
        QObject::tr("Follows"),
        QObject::tr("New followers")
    },
    {
        CHANNEL_CHAT,
        QObject::tr("Direct messages"),
        QObject::tr("New direct messages")
    },
    {
        CHANNEL_VERIFICATION,
        QObject::tr("Verification"),
        QObject::tr("Changes in your verification status")
    },
    {
        CHANNEL_ACTIVITY_SUBSCRIPTION,
        QObject::tr("Activity subscriptions"),
        QObject::tr("Posts from users you subscribed to")
    }
};

OffLineMessageChecker::OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp) :
    mNetwork(new QNetworkAccessManager(this)),
    mBackgroundApp(backgroundApp),
    mUserSettings(settingsFileName),
    mImageReader(mNetwork),
    mContentFilter(mUserPreferences, &mUserSettings),
    mMutedWords(mUserFollows),
    mNotificationListModel(mContentFilter, mBookmarks, mMutedWords)
{
    initNetwork();
    mNotificationListModel.enableRetrieveNotificationPosts(false);
}

OffLineMessageChecker::OffLineMessageChecker(const QString& settingsFileName, QEventLoop* eventLoop) :
    mNetwork(new QNetworkAccessManager(this)),
    mEventLoop(eventLoop),
    mUserSettings(settingsFileName),
    mImageReader(mNetwork),
    mContentFilter(mUserPreferences, &mUserSettings),
    mMutedWords(mUserFollows),
    mNotificationListModel(mContentFilter, mBookmarks, mMutedWords)
{
    initNetwork();
    mNotificationListModel.enableRetrieveNotificationPosts(false);
}

void OffLineMessageChecker::initNetwork()
{
    qDebug() << "Initialize network";
    Q_ASSERT(mNetwork);
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(30000);
}

int OffLineMessageChecker::startEventLoop()
{
    qDebug() << "Starting event loop";

    if (mBackgroundApp)
        return mBackgroundApp->exec();
    else if (mEventLoop)
        return mEventLoop->exec();

    return EXIT_OK;
}

void OffLineMessageChecker::exit(int exitCode)
{
    qDebug() << "Exit";

    if (mBackgroundApp)
        mBackgroundApp->exit(exitCode);
    else if (mEventLoop)
        mEventLoop->exit(exitCode);
}

int OffLineMessageChecker::check()
{
    const auto timestamp = mUserSettings.getOfflineMessageCheckTimestamp();
    qDebug() << "Previous check:" << timestamp;

    int exitStatus = EXIT_OK;

    // The network is not always available in a background task for some reason ???
    // Retry few times.
    for (int i = 0; i < 8; ++i)
    {
        qDebug() << "Attempt:" << i + 1;
        QTimer::singleShot(0, &mPresence, [this]{ resumeSession(); });
        exitStatus = startEventLoop();

        if (exitStatus == EXIT_OK)
        {
            mUserSettings.setOfflineMessageCheckTimestamp(QDateTime::currentDateTime());
            break;
        }

        if (exitStatus == EXIT_RETRY)
            break;

        if (exitStatus == EXIT_NETWORK_FAILURE)
        {
            exitStatus = EXIT_RETRY;
            std::this_thread::sleep_for(1s);
        }
    }

    return exitStatus;
}

void OffLineMessageChecker::createNotification(const QString channelId, const BasicProfile& author, const QString& msg, const QDateTime& when, IconType iconType)
{
    qDebug() << "Create notification:" << msg;

#if defined(Q_OS_ANDROID)
    QJniEnvironment env;
    QJniObject jChannelId = QJniObject::fromString(channelId);
    jint jNotificationId = mUserSettings.getNextNotificationId();
    QJniObject jTitle = QJniObject::fromString(author.getName());
    QJniObject jMsg = QJniObject::fromString(msg);
    jlong jWhen = when.toMSecsSinceEpoch();
    jint jIconType = (int)iconType;

    jbyteArray jAvatar = nullptr;
    const auto avatarUrl = author.getAvatarThumbUrl();

    if (!avatarUrl.isEmpty() && mAvatars.count(avatarUrl))
    {
        const QByteArray& jpg = mAvatars[avatarUrl];
        jAvatar = env->NewByteArray(jpg.size());
        env->SetByteArrayRegion(jAvatar, 0, jpg.size(), (jbyte*)jpg.data());
    }
    else
    {
        jAvatar = env->NewByteArray(0);
    }

    auto [javaClass, methodId] = JniUtils::getClassAndMethod(
        env,
        "com/gmail/mfnboer/NewMessageNotifier",
        "createNotification",
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;JI[B)V");

    if (!javaClass || !methodId)
        return;

    QJniObject::callStaticMethod<void>(
        javaClass,
        methodId,
        jChannelId.object<jstring>(),
        jNotificationId,
        jTitle.object<jstring>(),
        jMsg.object<jstring>(),
        jWhen,
        jIconType,
        jAvatar);
#else
    Q_UNUSED(channelId);
    Q_UNUSED(author);
    Q_UNUSED(msg);
    Q_UNUSED(when);
    Q_UNUSED(iconType);
#endif
}

std::optional<ATProto::ComATProtoServer::Session> OffLineMessageChecker::getSession() const
{
    const QString did = mUserSettings.getActiveUserDid();

    if (did.isEmpty())
        return {};

    const auto session = mUserSettings.getSession(did);

    if (session.mAccessJwt.isEmpty() || session.mRefreshJwt.isEmpty())
        return {};

    return session;
}

void OffLineMessageChecker::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    mUserSettings.saveSession(session);
    mUserSettings.sync();
}

void OffLineMessageChecker::resumeSession(bool retry)
{
    qDebug() << "Resume session, retry:" << retry;
    const auto session = getSession();

    if (!session)
    {
        qWarning() << "No saved session";
        exit(EXIT_OK);
        return;
    }

    mUserDid = session->mDid;

    // Need a longer network transfer timeout for an Android background process.
    auto xrpc = std::make_unique<Xrpc::Client>("", 30000);
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->resumeSession(*session,
        [this] {
            qDebug() << "Session resumed";
            saveSession(*mBsky->getSession());
            refreshSession();
        },
        [this, retry, session](const QString& error, const QString& msg){
            qWarning() << "Session could not be resumed:" << error << " - " << msg;

            if (!retry && error == ATProto::ATProtoErrorMsg::EXPIRED_TOKEN)
            {
                mBsky->setSession(std::make_shared<ATProto::ComATProtoServer::Session>(*session));
                mBsky->refreshSession(
                    [this]{
                        qDebug() << "Session refreshed";
                        saveSession(*mBsky->getSession());
                        resumeSession(true);
                    },
                    [this](const QString& error, const QString& msg){
                        qDebug() << "Session could not be refreshed:" << error << " - " << msg;
                        exit(EXIT_RETRY);
                    });
            }
            else if (error == ATProto::ATProtoErrorMsg::PDS_NOT_FOUND)
            {
                qWarning() << "PDS not found, network failure";
                exit(EXIT_NETWORK_FAILURE);
            }
            else
            {
                exit(EXIT_RETRY);
            }
        });
}

void OffLineMessageChecker::refreshSession()
{
    Q_ASSERT(mBsky);
    qDebug() << "Refresh session";

    const auto* session = mBsky->getSession();
    if (!session)
    {
        qWarning() << "No session to refresh.";
        exit(EXIT_OK);
        return;
    }

    mBsky->refreshSession(
        [this]{
            qDebug() << "Session refreshed";
            saveSession(*mBsky->getSession());
            getUserPreferences();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Session could not be refreshed:" << error << " - " << msg;
            exit(EXIT_RETRY);
        });
}

void OffLineMessageChecker::getUserPreferences()
{
    qDebug() << "Get user preferences";

    mBsky->getPreferences(
        [this](auto prefs){
            mUserPreferences = prefs;
            mMutedWords.load(mUserPreferences);
            getNotificationPreferences();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            exit(EXIT_RETRY);
        });
}

void OffLineMessageChecker::getNotificationPreferences()
{
    qDebug() << "Get notification preferences";

    mBsky->getNotificationPreferences(
        [this](auto prefs){
            mNotificationPrefs = prefs->mPreferences;
            checkUnreadNotificationCount();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            exit(EXIT_RETRY);
        });
}

void OffLineMessageChecker::checkUnreadNotificationCount()
{
    const int prevUnread = mUserSettings.getOfflineUnread(mUserDid);
    qDebug() << "Check unread notification count, last unread:" << prevUnread;

    mBsky->getUnreadNotificationCount({}, {},
        [this, prevUnread](int unread){
            qDebug() << "Unread notification count:" << unread;
            int newCount = unread - prevUnread;

            if (newCount < 0)
            {
                qDebug() << "Unread notification count has been reset by another client";
                newCount = unread;
                mUserSettings.setOfflineUnread(mUserDid, 0);
                mUserSettings.sync();
            }

            if (newCount == 0)
            {
                qDebug() << "No new posts";
                getChatNotifications();
                return;
            }

            getNotifications(newCount);
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Failed to get unread notification count:" << error << " - " << msg;
            getChatNotifications();
        });
}

void OffLineMessageChecker::getNotifications(int toRead)
{
    qDebug() << "Get notifications:" << toRead;
    const int limit = std::min(toRead, 100); // 100 is max that can be retreived in 1 request

    mBsky->listNotifications(limit, {}, {}, false, {},
        [this, toRead](auto notifications){
            filterNotifications(notifications);
            const bool added = mNotificationListModel.addNotifications(std::move(notifications), *mBsky, false,
                [this]{ getChatNotifications(); });

            const int prevUnread = mUserSettings.getOfflineUnread(mUserDid);
            mUserSettings.setOfflineUnread(mUserDid, prevUnread + toRead);
            mUserSettings.sync();

            if (!added)
                getChatNotifications();
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg;
            getChatNotifications();
        },
        false);
}

void OffLineMessageChecker::filterNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr notifications) const
{
    if (!notifications)
        return;

    for (auto it = notifications->mNotifications.begin(); it != notifications->mNotifications.end(); )
    {
        const auto& notification = *it;

        switch (notification->mReason)
        {
        case ATProto::AppBskyNotification::NotificationReason::LIKE:
            if (mNotificationPrefs->mLike && mNotificationPrefs->mLike->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::REPOST:
            if (mNotificationPrefs->mRepost && mNotificationPrefs->mRepost->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::FOLLOW:
            if (mNotificationPrefs->mFollow && mNotificationPrefs->mFollow->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::MENTION:
            if (mNotificationPrefs->mMention && mNotificationPrefs->mMention->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::REPLY:
            if (mNotificationPrefs->mReply && mNotificationPrefs->mReply->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::QUOTE:
            if (mNotificationPrefs->mQuote && mNotificationPrefs->mQuote->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::STARTERPACK_JOINED:
            if (mNotificationPrefs->mStarterpackJoined && mNotificationPrefs->mStarterpackJoined->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::VERIFIED:
            if (mNotificationPrefs->mVerified && mNotificationPrefs->mVerified->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::UNVERIFIED:
            if (mNotificationPrefs->mUnverified && mNotificationPrefs->mUnverified->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::LIKE_VIA_REPOST:
            if (mNotificationPrefs->mLikeViaRepost && mNotificationPrefs->mLikeViaRepost->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::REPOST_VIA_REPOST:
            if (mNotificationPrefs->mRepostViaRepost && mNotificationPrefs->mRepostViaRepost->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::SUBSCRIBED_POST:
            if (mNotificationPrefs->mSubscribedPost && mNotificationPrefs->mSubscribedPost->mPush)
            {
                ++it;
                continue;
            }

            break;
        case ATProto::AppBskyNotification::NotificationReason::UNKNOWN:
            break;
        }

        qDebug() << "Remove push notification:" << notification->mRawReason;
        it = notifications->mNotifications.erase(it);
    }
}

void OffLineMessageChecker::getChatNotifications()
{
    qDebug() << "Get chat notifications";

    if (!mUserSettings.mustCheckOfflineChat(mUserDid) || (mNotificationPrefs && mNotificationPrefs->mChat && !mNotificationPrefs->mChat->mPush))
    {
        qDebug() << "Chat not enabled";
        getAvatars();
    }

    // We used to list only unread convos, but that will not get convos with
    // new reactions.
    mBsky->listConvos({}, false, {}, {},
        [this](ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr output){
            const QString lastRev = mUserSettings.getOffLineChatCheckRev(mUserDid);
            const QString rev = mNotificationListModel.addNotifications(std::move(output), lastRev, mUserDid);

            if (!rev.isNull() && rev > lastRev)
                mUserSettings.setOffLineChatCheckRev(mUserDid, rev);

            getAvatars();
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getChatNotifications FAILED:" << error << " - " << msg;
            getAvatars();
        }
    );
}

void OffLineMessageChecker::getAvatars()
{
    std::set<QString> avatarUrls;
    const auto& notifications = mNotificationListModel.getNotifications();

    if (notifications.empty())
    {
        qDebug() << "No notifications";
        exit(EXIT_OK);
    }

    for (const auto& notification : notifications)
    {
        const QString url = notification.getAuthor().getAvatarThumbUrl();

        if (!url.isEmpty())
            avatarUrls.insert(url);
    }

    const QStringList urls(avatarUrls.begin(), avatarUrls.end());
    getAvatars(urls);
}

void OffLineMessageChecker::getAvatars(const QStringList& urls)
{
    if (urls.isEmpty())
    {
        createNotifications();
        return;
    }

    const QString url = urls.front();
    const auto remainingUrls = urls.mid(1);
    qDebug() << "Get avatar:" << url;

    const bool validUrl = mImageReader.getImage(url,
        [this, url, remainingUrls](QImage image){
            QByteArray jpgBlob;
            PhotoPicker::createBlob(jpgBlob, image);

            if (!jpgBlob.isNull())
                mAvatars[url] = jpgBlob;
            else
                qWarning() << "Could not convert avatar to JPG:" << url;

            getAvatars(remainingUrls);
        },
        [this, remainingUrls](const QString& error){
            qWarning() << "Failed to get avatar:" << error;
            getAvatars(remainingUrls);
        });

    if (!validUrl)
        getAvatars(remainingUrls);
}

void OffLineMessageChecker::createNotifications()
{
    const auto& notifications = mNotificationListModel.getNotifications();
    qDebug() << "Create notifications:" << notifications.size();

    for (const auto& notification : notifications)
        createNotification(notification);

    exit(EXIT_OK);
}

void OffLineMessageChecker::createNotification(const Notification& notification)
{
    const PostCache& reasonPostCache = mNotificationListModel.getReasonPostCache();
    const PostRecord postRecord = notification.getPostRecord();

    // NOTE: postText can be empty if there is only an image.
    QString msg = !postRecord.isNull() ? postRecord.getFormattedText() : "";
    QString channelId = CHANNEL_POST;
    IconType iconType = IconType::POST;

    switch (notification.getReason())
    {
    case Notification::Reason::NOTIFICATION_REASON_LIKE:
    case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
    {
        channelId = CHANNEL_LIKE;
        iconType = IconType::LIKE;
        msg = notification.getReason() == Notification::Reason::NOTIFICATION_REASON_LIKE ?
                QObject::tr("<b>Liked your post</b>") :
                QObject::tr("<b>Liked your repost</b>");
        const Post post = notification.getReasonPost(reasonPostCache);
        const auto reasonPostText = post.getFormattedText();

        if (!reasonPostText.isEmpty())
            msg.append("<br>" + reasonPostText);

        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_REPOST:
    case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
    {
        channelId = CHANNEL_REPOST;
        iconType = IconType::REPOST;
        msg = notification.getReason() == Notification::Reason::NOTIFICATION_REASON_REPOST ?
                QObject::tr("<b>Reposted your post</b>") :
                QObject::tr("<b>Reposted your repost</b>");
        const Post post = notification.getReasonPost(reasonPostCache);
        const auto reasonPostText = post.getFormattedText();

        if (!reasonPostText.isEmpty())
            msg.append("<br>" + reasonPostText);

        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_FOLLOW:
        channelId = CHANNEL_FOLLOW;
        iconType = IconType::FOLLOW;
        msg = QObject::tr("<b>Follows you</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_MENTION:
        iconType = IconType::MENTION;
        msg = QObject::tr("<b>Mentioned you</b><br>") + msg;
        break;
    case Notification::Reason::NOTIFICATION_REASON_REPLY:
        msg = QObject::tr("<b>Replied to your post</b><br>") + msg;
        break;
    case Notification::Reason::NOTIFICATION_REASON_QUOTE:
        msg = QObject::tr("<b>Quoted your post</b><br>") + msg;
        break;
    case Notification::Reason::NOTIFICATION_REASON_SUBSCRIBED_POST:
        msg = QObject::tr("<b>Subscribed post</b><br>") + msg;
        channelId = CHANNEL_ACTIVITY_SUBSCRIPTION;
        break;
    case Notification::Reason::NOTIFICATION_REASON_DIRECT_MESSAGE:
        channelId = CHANNEL_CHAT;
        iconType = IconType::CHAT;
        msg = QObject::tr("<b>Direct message</b><br>") + notification.getDirectMessage().getFormattedText();
        break;
    case Notification::Reason::NOTIFICATION_REASON_DIRECT_MESSAGE_REACTION:
    {
        channelId = CHANNEL_CHAT;
        iconType = IconType::CHAT;
        const auto& msgAndReaction = notification.getDirectMessageAndReaction();
        msg = QObject::tr("Reacts with %1 to: %2").arg(
                msgAndReaction.getReactionView().getEmoji(),
                msgAndReaction.getMessageView().getFormattedText());
        break;
    }
    case Notification::Reason::NOTIFICATION_REASON_VERIFIED:
        channelId = CHANNEL_VERIFICATION;
        iconType = IconType::VERIFICATION;
        msg = QObject::tr("<b>Verified you</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_UNVERIFIED:
        channelId = CHANNEL_VERIFICATION;
        iconType = IconType::VERIFICATION;
        msg = QObject::tr("<b>Deleted your verification</b>");
        break;
    case Notification::Reason::NOTIFICATION_REASON_STARTERPACK_JOINED:
    case Notification::Reason::NOTIFICATION_REASON_INVITE_CODE_USED:
    case Notification::Reason::NOTIFICATION_REASON_NEW_LABELS:
    case Notification::Reason::NOTIFICATION_REASON_UNKNOWN:
        return;
    }

    QDateTime when = notification.getTimestamp();

    if (when.isNull())
        when = QDateTime::currentDateTimeUtc();

    createNotification(channelId, notification.getAuthor(), msg, when, iconType);
}

void OffLineMessageChecker::checkNotificationPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString POST_NOTIFICATIONS = "android.permission.POST_NOTIFICATIONS";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion < QOperatingSystemVersion::Android13)
    {
        qDebug() << "Android version:" << osVersion;
        sNotificationPermissionGranted = true;
    }

    if (!AndroidUtils::checkPermission(POST_NOTIFICATIONS))
    {
        qDebug() << "No permission:" << POST_NOTIFICATIONS;
        sNotificationPermissionGranted = false;
    }

    sNotificationPermissionGranted = true;
#else
    sNotificationPermissionGranted = false;
#endif
}

void OffLineMessageChecker::start(bool wifiOnly)
{
#if defined(Q_OS_ANDROID)
    // Do not peroform the permission check here. It seems that sometimes
    // crashes when the app is going into pause mode.
    if (!sNotificationPermissionGranted)
    {
        qDebug() << "No permission to send notifications";
        return;
    }

    qDebug() << "Start offline message checker";
    jboolean jWifiOnly = wifiOnly;

    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/NewMessageChecker",
        "startChecker",
        "(Z)V",
        jWifiOnly);
#else
    Q_UNUSED(wifiOnly);
#endif
}

void OffLineMessageChecker::createNotificationChannels()
{
#if defined(Q_OS_ANDROID)
    QJniEnvironment env;

    for (const auto& channel : NOTIFCATION_CHANNELS)
    {
        QJniObject jId = QJniObject::fromString(channel.mId);
        QJniObject jName = QJniObject::fromString(channel.mName);
        QJniObject jDescription = QJniObject::fromString(channel.mDescription);

        QJniObject::callStaticMethod<void>(
            "com/gmail/mfnboer/NewMessageNotifier",
            "createNotificationChannel",
            "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            jId.object<jstring>(),
            jName.object<jstring>(),
            jDescription.object<jstring>());
    }
#endif
}

}
