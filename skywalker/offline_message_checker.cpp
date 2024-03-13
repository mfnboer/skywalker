// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "offline_message_checker.h"

#ifdef Q_OS_ANDROID
#include "jni_utils.h"
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#include <QTimer>
#endif

#if defined(Q_OS_ANDROID)
JNIEXPORT void JNICALL Java_com_gmail_mfnboer_NewMessageChecker_checkNewMessages(JNIEnv* env, jobject, jstring jSettingsFileName, jstring jLibDir)
{
    qSetMessagePattern("%{time HH:mm:ss.zzz} %{type} %{function}'%{line} %{message}");
    QJniEnvironment::checkAndClearExceptions(env);
    QJniEnvironment jniEnv;

    bool equalEnv = jniEnv.jniEnv() == env;
    qDebug() << "JNI CHECK:" << equalEnv << jniEnv.jniEnv() << env;

    qDebug() << "CHECK NEW MESSAGES START";

    const char* settingsFileName = (*env).GetStringUTFChars(jSettingsFileName, nullptr);

    if (!settingsFileName)
    {
        qWarning() << "Settings file name missing";
        return;
    }

    const char* libDir = (*env).GetStringUTFChars(jLibDir, nullptr);

    if (!libDir)
    {
        qWarning() << "Library directory missing";
        return;
    }

    std::unique_ptr<QCoreApplication> app;

    if (!QCoreApplication::instance())
    {
        qDebug() << "Start core app";
        QCoreApplication::addLibraryPath(libDir);
        int argc = 1;
        const char* argv[] = {"com.gmail.mfnboer.skywalker", NULL};
        app = std::make_unique<QCoreApplication>(argc, (char**)argv);
        qDebug() << "LIBS:" << app->libraryPaths();
    }
    else
    {
        qDebug() << "App still running";
    }

    // This makes networking work???
    // Somehow Qt fails to find this class if we don't do this lookup.
    jclass javaClass = jniEnv.findClass("org/qtproject/qt/android/network/QtNetwork");
    qDebug() << "QtNework class:" << javaClass;

    auto* checker = new Skywalker::OffLineMessageChecker(settingsFileName, app.get());

    if (app)
    {
        QObject guard;
        QTimer::singleShot(0, &guard, [checker]{
            // Pass explicit file name as Qt does not know the app data path when running in
            // a background task.
            checker->run();
        });

        app->exec();
        delete checker;
    }
    else
    {
        checker->run();
        delete checker;
    }

    (*env).ReleaseStringUTFChars(jSettingsFileName, settingsFileName);
    (*env).ReleaseStringUTFChars(jLibDir, libDir);
    qDebug() << "CHECK NEW MESSAGES END";
}
#endif

namespace Skywalker {

OffLineMessageChecker::OffLineMessageChecker(const QString& settingsFileName, QCoreApplication* backgroundApp) :
    mBackgroundApp(backgroundApp),
    mUserSettings(settingsFileName)
{}

void OffLineMessageChecker::exit()
{
    if (mBackgroundApp)
        mBackgroundApp->exit();
}

void OffLineMessageChecker::run()
{
    if (mBackgroundApp)
        resumeSession();
    else
        checkNewMessages();
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

    qDebug() << "Starting event loop";

    if (mBackgroundApp)
        mBackgroundApp->exec();
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
            checkNewMessages();
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
        [this, host, password]{
            qDebug() << "Login" << mUserDid << "succeeded";
            const auto* session = mBsky->getSession();
            saveSession(*session);
            checkNewMessages();
        },
        [this, host](const QString& error, const QString& msg){
            qDebug() << "Login" << mUserDid << "failed:" << error << " - " << msg;
            exit();
        });
}

}
