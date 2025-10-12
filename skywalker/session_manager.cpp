// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "session_manager.h"
#include "skywalker.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto SESSION_REFRESH_DELAY = 11s;
static constexpr auto NOTIFICATION_REFRESH_DELAY = 5s;
static constexpr auto NOTIFICATION_REFRESH_INTERVAL = 61s;

SessionManager::SessionManager(UserSettings& userSettings, QObject* parent) :
    QObject(parent),
    mUserSettings(userSettings)
{
}

void SessionManager::resumeAndRefreshNonActiveUsers()
{
    const auto activeDid = mUserSettings.getActiveUserDid();
    const auto dids = mUserSettings.getUserDidList();

    for (const auto& did : dids)
    {
        if (did != activeDid)
            resumeAndRefreshSession(did);
    }
}

void SessionManager::clear()
{
    mDidSessionMap.clear();
}

bool SessionManager::resumeAndRefreshSession(const QString& did)
{
    qDebug() << "Resume and refresh session:" << did;
    const auto session = getSavedSession(did);

    if (!session)
    {
        qDebug() << "No saved session:" << did;
        return false;
    }

    qDebug() << "Session:" << session->mDid << session->mAccessJwt << session->mRefreshJwt;

    auto xrpc = std::make_unique<Xrpc::Client>();
    xrpc->setUserAgent(Skywalker::getUserAgentString());

    Session::Ptr managedSession = createSession(did);
    managedSession->mBsky = std::make_unique<ATProto::Client>(std::move(xrpc), this);
    mDidSessionMap[did] = std::move(managedSession);
    const int refreshDelayCount = mDidSessionMap.size();

    mDidSessionMap[did]->mBsky->resumeAndRefreshSession(*session,
        [this, did, refreshDelayCount]{
            qDebug() << "Session resumed:" << did;
            auto* session = getSession(did);

            if (!session)
                return;

            auto& bsky = session->mBsky;
            mUserSettings.saveSession(*bsky->getSession());
            mUserSettings.sync();
            startRefreshTimers(did, refreshDelayCount);
        },
        [this, did](const QString& error, const QString& msg){
            qDebug() << "Session could not be resumed:" << error << " - " << msg << "did:" << did;

            if (error == ATProto::ATProtoErrorMsg::REFRESH_SESSION_FAILED)
                mUserSettings.clearTokens(did); // calls sync

            mDidSessionMap.erase(did);
        });


    return true;
}

void SessionManager::startRefreshTimers()
{
    int refreshDelayCount = 1;

    for (const auto& [did, _] : mDidSessionMap)
        startRefreshTimers(did, refreshDelayCount++);
}

void SessionManager::stopRefreshTimers()
{
    for (const auto& [did, _] : mDidSessionMap)
        stopRefreshTimers(did);
}

void SessionManager::saveTokens()
{
    for (const auto& [_, session] : mDidSessionMap)
    {
        if (session->mBsky)
            mUserSettings.saveSession(*session->mBsky->getSession());
    }

    mUserSettings.sync();
}

void SessionManager::updateTokens()
{
    for (const auto& [did, session] : mDidSessionMap)
    {
        auto savedSession = mUserSettings.getSession(did);

        if (!session->mBsky)
            continue;

        if (!savedSession.mRefreshJwt.isEmpty())
            session->mBsky->updateTokens(savedSession.mAccessJwt, savedSession.mRefreshJwt);
        else
            qWarning() << "No tokens:" << did;
    }
}

SessionManager::Session::Ptr SessionManager::createSession(const QString& did)
{
    auto session = std::make_unique<Session>();
    session->mRefreshNotificationInitialDelayTimer.setSingleShot(true);
    connect(&session->mRefreshNotificationInitialDelayTimer, &QTimer::timeout, this, [this, did]{
        auto* session = getSession(did);

        if (session)
            session->mRefreshNotificationTimer.start(NOTIFICATION_REFRESH_INTERVAL);
    });
    connect(&session->mRefreshNotificationTimer, &QTimer::timeout, this, [this, did]{
        refreshNotificationCount(did);
    });
    return session;
}

SessionManager::Session* SessionManager::getSession(const QString& did)
{
    if (!mDidSessionMap.contains(did))
        return nullptr;

    auto& session = mDidSessionMap[did];

    if (!session->mBsky)
    {
        qWarning() << "Session is not active:" << did;
        return nullptr;
    }

    return session.get();
}

std::optional<ATProto::ComATProtoServer::Session> SessionManager::getSavedSession(const QString& did) const
{
    if (did.isEmpty())
    {
        qWarning() << "Empty DID";
        return {};
    }

    const auto session = mUserSettings.getSession(did);

    if (session.mAccessJwt.isEmpty() || session.mRefreshJwt.isEmpty())
    {
        qDebug() << "No JWT:" << did;
        return {};
    }

    return session;
}

void SessionManager::startRefreshTimers(const QString& did, int initialDelayCount)
{
    qDebug() << "Start refresh timers:" << did;
    auto* session = getSession(did);

    if (!session)
        return;

    session->mBsky->startAutoRefresh(initialDelayCount * SESSION_REFRESH_DELAY,
        [this, did]{
            auto* session = getSession(did);

            if (!session)
                return;

            mUserSettings.saveSession(*session->mBsky->getSession());
            mUserSettings.syncLater();
        },
        [this, did](const QString& msg){
            qWarning() << "Session refresh failed:" << msg;
            mDidSessionMap.erase(did);
        });

    session->mRefreshNotificationInitialDelayTimer.start(initialDelayCount * NOTIFICATION_REFRESH_DELAY);
}

void SessionManager::stopRefreshTimers(const QString& did)
{
    qDebug() << "Stop refresh timers:" << did;
    auto* session = getSession(did);

    if (!session)
        return;

    session->mBsky->stopAutoRefresh();
    session->mRefreshNotificationInitialDelayTimer.stop();
    session->mRefreshNotificationTimer.stop();
}

void SessionManager::refreshNotificationCount(const QString& did)
{
    qDebug() << "Refresh notification count:" << did;
    auto* session = getSession(did);

    if (!session)
        return;

    session->mBsky->getUnreadNotificationCount({}, {},
        [this, did](int unread){
            qDebug() << "Unread notification count:" << unread << did;
            setUnreadNotificationCount(did, unread);
        },
        [did](const QString& error, const QString& msg){
            qWarning() << "Failed to get unread notification count:" << error << " - " << msg << "did:" << did;
        });
}

void SessionManager::setUnreadNotificationCount(const QString& did, int unread)
{
    auto* session = getSession(did);

    if (!session)
        return;

    session->mUnreadNotificationCount = unread;
}

}
