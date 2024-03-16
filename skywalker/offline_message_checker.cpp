// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "offline_message_checker.h"
#include "notification.h"

#ifdef Q_OS_ANDROID
#include "android_utils.h"
#include "jni_utils.h"
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#include <QTimer>
#endif

#if defined(Q_OS_ANDROID)
JNIEXPORT int JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv* env, jobject, jint unread, jstring jSettingsFileName, jstring jLibDir)
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");
    qDebug() << "CHECK NEW MESSAGES START, UNREAD:" << unread;

    const char* settingsFileName = (*env).GetStringUTFChars(jSettingsFileName, nullptr);

    if (!settingsFileName)
    {
        qWarning() << "Settings file name missing";
        return 0;
    }

    const char* libDir = (*env).GetStringUTFChars(jLibDir, nullptr);

    if (!libDir)
    {
        qWarning() << "Library directory missing";
        return 0;
    }

    // This makes networking work???
    // Somehow Qt fails to find this class if we don't do this lookup.
    QJniEnvironment jniEnv;
    jclass javaClass = jniEnv.findClass("org/qtproject/qt/android/network/QtNetwork");

    if (!javaClass)
    {
        qWarning() << "Class loading failed";
        return 0;
    }

    std::unique_ptr<QCoreApplication> app;
    std::unique_ptr<QEventLoop> eventLoop;
    std::unique_ptr<Skywalker::OffLineMessageChecker> checker;

    if (!QCoreApplication::instance())
    {
        qDebug() << "Start core app";
        QCoreApplication::addLibraryPath(libDir);
        int argc = 1;
        const char* argv[] = {"Skywalker", NULL};
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

    checker->setPrevUnreadCount(unread);
    checker->check();

    (*env).ReleaseStringUTFChars(jSettingsFileName, settingsFileName);
    (*env).ReleaseStringUTFChars(jLibDir, libDir);
    qDebug() << "CHECK NEW MESSAGES END, UNREAD:" << checker->getPrevUnreadCount();

    return checker->getPrevUnreadCount();
}
#endif

namespace Skywalker {

OffLineMessageChecker::OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp) :
    mBackgroundApp(backgroundApp),
    mUserSettings(settingsFileName)
{
}

OffLineMessageChecker::OffLineMessageChecker(const QString& settingsFileName, QEventLoop* eventLoop) :
    mEventLoop(eventLoop),
    mUserSettings(settingsFileName)
{
}

void OffLineMessageChecker::startEventLoop()
{
    qDebug() << "Starting event loop";

    if (mBackgroundApp)
        mBackgroundApp->exec();
    else if (mEventLoop)
        mEventLoop->exec();
}

void OffLineMessageChecker::exit()
{
    qDebug() << "Exit";

    if (mBackgroundApp)
        mBackgroundApp->exit();
    else if (mEventLoop)
        mEventLoop->exit();
}

void OffLineMessageChecker::check()
{
    QObject guard;
    QTimer::singleShot(0, &guard, [this]{ resumeSession(); });
    startEventLoop();
}

void OffLineMessageChecker::checkNewMessages()
{
#if defined(Q_OS_ANDROID)
    qDebug() << "Check messages for:" << mUserDid;
    createNotification("Michel Bestaat", "Test post");
    exit();
#endif
}

void OffLineMessageChecker::createNotification(const QString& title, const QString& msg)
{
    qDebug() << "Create notification:" << title << msg;

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

bool OffLineMessageChecker::getSession(QString& host, ATProto::ComATProtoServer::Session& session)
{
    const QString did = mUserSettings.getActiveUserDid();

    if (did.isEmpty())
        return false;

    session = mUserSettings.getSession(did);

    if (session.mAccessJwt.isEmpty() || session.mRefreshJwt.isEmpty())
        return false;

    host = mUserSettings.getHost(did);

    if (host.isEmpty())
        return false;

    return true;
}

void OffLineMessageChecker::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    mUserSettings.saveSession(session);
}

void OffLineMessageChecker::resumeSession()
{
    qDebug() << "Resume session";
    QString host;
    ATProto::ComATProtoServer::Session session;

    if (!getSession(host, session))
    {
        qWarning() << "No saved session";
        exit();
        return;
    }

    mUserDid = session.mDid;
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->resumeSession(session,
        [this] {
            qDebug() << "Session resumed";
            saveSession(*mBsky->getSession());
            refreshSession();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Session could not be resumed:" << error << " - " << msg;

            if (error == "ExpiredToken")
                login();
            else
                exit();
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
        exit();
        return;
    }

    mBsky->refreshSession(
        [this]{
            qDebug() << "Session refreshed";
            saveSession(*mBsky->getSession());
            checkUnreadNotificationCount();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Session could not be refreshed:" << error << " - " << msg;
            exit();
        });
}

void OffLineMessageChecker::login()
{
    const auto host = mUserSettings.getHost(mUserDid);
    const auto password = mUserSettings.getPassword(mUserDid);

    if (host.isEmpty() || password.isEmpty())
    {
        qWarning() << "Missing host or password to login";
        exit();
        return;
    }

    qDebug() << "Login:" << mUserDid << "host:" << host;
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->createSession(mUserDid, password,
        [this]{
            qDebug() << "Login" << mUserDid << "succeeded";
            const auto* session = mBsky->getSession();
            saveSession(*session);
            checkUnreadNotificationCount();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Login" << mUserDid << "failed:" << error << " - " << msg;
            exit();
        });
}

void OffLineMessageChecker::checkUnreadNotificationCount()
{
    qDebug() << "Check unread notification count";

    mBsky->getUnreadNotificationCount({},
        [this](int unread){
            qDebug() << "Unread notification count:" << unread;

            if (unread < mPrevUnreadCount)
            {
                qDebug() << "Unread notification count has been reset by another client";
                mPrevUnreadCount = 0;
            }

            if (unread == mPrevUnreadCount)
            {
                qDebug() << "No new messages";
                exit();
                return;
            }

            getNotifications(unread);
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Failed to get unread notification count:" << error << " - " << msg;
            exit();
        });
}

void OffLineMessageChecker::getNotifications(int unread)
{
    qDebug() << "Get notifications, unread:" << unread << "prev:" << mPrevUnreadCount;

    if (unread <= mPrevUnreadCount)
    {
        qWarning() << "No new messages!!";
        mPrevUnreadCount = unread;
        exit();
        return;
    }

    // TODO: maximize? cannot get more than 100
    const int msgCount = unread - mPrevUnreadCount;

    mBsky->listNotifications(msgCount, {}, {},
        [this, unread](auto notifications){
            createNotifications(notifications->mNotifications);
            mPrevUnreadCount = unread;
            exit();
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg;
            exit();
        },
        false);
}

void OffLineMessageChecker::createNotifications(const ATProto::AppBskyNotification::NotificationList& notifications)
{
    qDebug() << "Create notifications:" << notifications.size();

    for (const auto& notification : notifications)
        createNotification(notification.get());
}

void OffLineMessageChecker::createNotification(const ATProto::AppBskyNotification::Notification* protoNotification)
{
    const Notification notification(protoNotification);
    const PostRecord postRecord = notification.getPostRecord();
    const QString postText = !postRecord.isNull() ? postRecord.getText() : "";
    QString msg;

    switch (notification.getReason())
    {
    case ATProto::AppBskyNotification::NotificationReason::LIKE:
        msg = QObject::tr("Liked your post\n") + postText; // TODO: post is not in record
        break;
    case ATProto::AppBskyNotification::NotificationReason::REPOST:
        msg = QObject::tr("Reposted your post\n") + postText; // TODO: post is not in record
        break;
    case ATProto::AppBskyNotification::NotificationReason::FOLLOW:
        msg = QObject::tr("Follows you");
        break;
    case ATProto::AppBskyNotification::NotificationReason::MENTION:
        msg = postText;
        break;
    case ATProto::AppBskyNotification::NotificationReason::REPLY:
        msg = postText;
        break;
    case ATProto::AppBskyNotification::NotificationReason::QUOTE:
        msg = postText;
        break;
    case ATProto::AppBskyNotification::NotificationReason::INVITE_CODE_USED:
    case ATProto::AppBskyNotification::NotificationReason::UNKNOWN:
        return;
    }

    // TODO: truncate message? Number of lines?
    createNotification(notification.getAuthor().getName(), msg);
}

bool OffLineMessageChecker::checkNoticationPermission()
{
#if defined(Q_OS_ANDROID)
    static constexpr char const* POST_NOTIFICATIONS = "android.permission.POST_NOTIFICATIONS";

    if (!AndroidUtils::checkPermission(POST_NOTIFICATIONS))
    {
        qDebug() << "No permission:" << POST_NOTIFICATIONS;
        return false;
    }

    return true;
#else
    return false;
#endif
}

void OffLineMessageChecker::start()
{
#if defined(Q_OS_ANDROID)
    if (!checkNoticationPermission())
        return;

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/NewMessageChecker", "startChecker");
#endif
}

void OffLineMessageChecker::createNotificationChannel()
{
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/NewMessageNotifier", "createNotificationChannel");
#endif
}

}
