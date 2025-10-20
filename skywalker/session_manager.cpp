// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "session_manager.h"
#include "skywalker.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto SESSION_REFRESH_DELAY = 11s;
static constexpr auto NOTIFICATION_REFRESH_DELAY = 5s;
static constexpr auto NOTIFICATION_REFRESH_INTERVAL = 61s;
static constexpr auto NOTIFICATION_REFRESH_ACTIVE_USER_INTERVAL = 29s;

SessionManager::SessionManager(QObject* parent) :
    QObject(parent)
{
}

SessionManager::SessionManager(Skywalker* skywalker, QObject* parent) :
    QObject(parent),
    mSkywalker(skywalker),
    mUserSettings(skywalker->getUserSettings())
{
    Q_ASSERT(skywalker);

    connect(mUserSettings, &UserSettings::notificationsForAllAccountsChanged, this, [this]{
        if (mUserSettings->getNotificationsForAllAccounts(mSkywalker->getUserDid()))
            enableNotificationsNonActiveUsers();
        else
            disableNotificatiosNonActiveUsers();

        emit nonActiveNotificationsChanged();
    });
}

void SessionManager::resumeAndRefreshNonActiveUsers()
{
    const auto activeDid = mSkywalker->getUserDid();
    const auto dids = mUserSettings->getUserDidList();

    for (const auto& did : dids)
    {
        if (did != activeDid)
            resumeAndRefreshSession(did);
    }
}

void SessionManager::clear()
{
    mNonActiveUsers.clear();
    mExpiredUsers.clear();
    mDidSessionMap.clear();

    emit nonActiveUsersChanged();

    // All sessions are cleared, so count is now zero.
    emit activeUserUnreadNotificationCountChanged();

    if (mUserSettings->getNotificationsForAllAccounts(mSkywalker->getUserDid()))
        emit nonActiveNotificationsChanged();
}

void SessionManager::insertSession(const QString& did, ATProto::Client* client)
{
    Session::Ptr managedSession = createSession(did, nullptr, client);
    insertSession(did, std::move(managedSession));
}

bool SessionManager::resumeAndRefreshSession(const QString& did)
{
    qDebug() << "Resume and refresh session:" << did;
    const auto session = getSavedSession(did);

    if (!session)
    {
        qDebug() << "No saved session:" << did;
        addExpiredUser(did);
        return false;
    }

    qDebug() << "Session:" << session->mDid << session->mAccessJwt << session->mRefreshJwt;

    auto xrpc = std::make_unique<Xrpc::Client>();
    xrpc->setUserAgent(Skywalker::getUserAgentString());

    auto rawBsky = std::make_unique<ATProto::Client>(std::move(xrpc), this);

    // NOTE: we set the labelers from the active user. So the posts you see
    // for your other accounts will be labeled by the labelers you subscribed to
    // with your active account.
    const auto& dids = mSkywalker->getContentFilter()->getSubscribedLabelerDids();

    auto* bsky = rawBsky.get();
    Session::Ptr managedSession = createSession(did, std::move(rawBsky), bsky);
    insertSession(did, std::move(managedSession));

    const int refreshDelayCount = mDidSessionMap.size() - 1;
    resumeAndRefreshSession(mDidSessionMap[did]->mBsky, *session, refreshDelayCount);
    return true;
}

void SessionManager::resumeAndRefreshSession(ATProto::Client* client, const ATProto::ComATProtoServer::Session& session,
                                             int refreshDelayCount, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    const QString did = session.mDid;

    client->resumeAndRefreshSession(session,
        [this, did, refreshDelayCount, successCb, errorCb]{
            qDebug() << "Session resumed:" << did;
            auto* session = getSession(did);

            if (!session)
            {
                if (errorCb)
                    errorCb("NoSession", "No session");

                return;
            }

            auto& bsky = session->mBsky;
            mUserSettings->saveSession(*bsky->getSession());
            mUserSettings->sync();

            // Timers for the active user are started by Skywalker::resumeAndRefreshSession()
            if (did != mSkywalker->getUserDid())
                startRefreshTimers(did, refreshDelayCount);

            if (successCb)
                successCb();
        },
        [this, did, errorCb](const QString& error, const QString& msg){
            qDebug() << "Session could not be resumed:" << error << " - " << msg << "did:" << did;

            if (error == ATProto::ATProtoErrorMsg::REFRESH_SESSION_FAILED)
                mUserSettings->clearTokens(did); // calls sync

            deleteSession(did);

            if (errorCb)
                errorCb(error, msg);
        });
}

void SessionManager::startRefreshTimers()
{
    int refreshDelayCount = 1;

    for (const auto& [did, _] : mDidSessionMap)
    {
        if (did == mSkywalker->getUserDid())
            startRefreshTimers(did, 0);
        else
            startRefreshTimers(did, refreshDelayCount++);
    }
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
        {
            auto* atprotoSession = session->mBsky->getSession();

            if (atprotoSession)
                mUserSettings->saveSession(*atprotoSession);
        }
    }

    mUserSettings->sync();
}

void SessionManager::updateTokens()
{
    for (const auto& [did, session] : mDidSessionMap)
    {
        auto savedSession = mUserSettings->getSession(did);

        if (!session->mBsky || !session->mBsky->getSession())
            continue;

        qDebug() << "Update tokens:" << did;

        // The offline message checker may have refreshed tokens. Update these tokens
        // so we do not use an old token for refreshing below.
        if (!savedSession.mRefreshJwt.isEmpty())
            session->mBsky->updateTokens(savedSession.mAccessJwt, savedSession.mRefreshJwt);
        else
            qWarning() << "No tokens:" << did;
    }
}

SessionManager::Session::Ptr SessionManager::createSession(const QString& did, ATProto::Client::SharedPtr rawBsky, ATProto::Client* bsky)
{
    Q_ASSERT(!mSkywalker->getUserDid().isEmpty());
    auto session = std::make_unique<Session>();
    session->mSharedBsky = rawBsky;
    session->mBsky = bsky;
    session->mRefreshNotificationInitialDelayTimer.setSingleShot(true);

    if (did != mSkywalker->getUserDid())
    {
        const BasicProfile profile = mUserSettings->getUser(did);

        if (!profile.getHandle().isEmpty())
        {
            session->mNonActiveUser = std::make_unique<NonActiveUser>(
                profile, false, session->mSharedBsky, this, this);
        }
        else
        {
            qWarning() << "Invalid user:" << did;
        }
    }

    connect(&session->mRefreshNotificationInitialDelayTimer, &QTimer::timeout, this, [this, did]{
        auto* session = getSession(did);

        if (session)
        {
            if (did == mSkywalker->getUserDid())
                session->mRefreshNotificationTimer.start(NOTIFICATION_REFRESH_ACTIVE_USER_INTERVAL);
            else
                session->mRefreshNotificationTimer.start(NOTIFICATION_REFRESH_INTERVAL);
        }
    });

    connect(&session->mRefreshNotificationTimer, &QTimer::timeout, this, [this, did]{
        refreshNotificationCount(did);
    });

    return session;
}

void SessionManager::insertSession(const QString& did, Session::Ptr session)
{
    auto* nonActiveUser = session->mNonActiveUser.get();
    mDidSessionMap[did] = std::move(session);

    if (!nonActiveUser)
        return;

    addNonActiveUser(nonActiveUser);
}

void SessionManager::deleteSession(const QString& did)
{
    if (!mDidSessionMap.contains(did))
        return;

    qDebug() << "Delete session:" << did;

    // Do not destroy the active user, it can be in use e.g. in a list of non-active
    // users showing in the UI.
    auto& session = mDidSessionMap[did];
    NonActiveUser::Ptr nonActiveUser = std::move(session->mNonActiveUser);
    mDidSessionMap.erase(did);

    if (!nonActiveUser)
        return;

    nonActiveUser->expireSession();
    mExpiredUsers.push_back(std::move(nonActiveUser));

    qDebug() << "Session expired:" << did;

    if (mUserSettings->getNotificationsForAllAccounts(mSkywalker->getUserDid()))
        emit nonActiveNotificationsChanged();
}

void SessionManager::addNonActiveUser(NonActiveUser* nonActiveUser)
{
    Q_ASSERT(nonActiveUser);

    if (!nonActiveUser)
        return;

    mNonActiveUsers.push_back(nonActiveUser);

    std::sort(mNonActiveUsers.begin(), mNonActiveUsers.end(),
              [](const NonActiveUser* lhs, const NonActiveUser* rhs){
                  return lhs->getProfile().getHandle() < rhs->getProfile().getHandle();
              });

    emit nonActiveUsersChanged();

    if (mUserSettings->getNotificationsForAllAccounts(mSkywalker->getUserDid()))
        emit nonActiveNotificationsChanged();
}

void SessionManager::addExpiredUser(const QString& did)
{
    BasicProfile profile = mUserSettings->getUser(did);

    if (profile.getHandle().isEmpty())
    {
        qWarning() << "Invalid user:" << did;
        return;
    }

    auto expiredUser = std::make_unique<NonActiveUser>(profile, true, nullptr, this, this);
    mExpiredUsers.push_back(std::move(expiredUser));
    addNonActiveUser(mExpiredUsers.back().get());
}

SessionManager::Session* SessionManager::getSession(const QString& did) const
{
    if (!mDidSessionMap.contains(did))
        return nullptr;

    auto& session = mDidSessionMap.at(did);

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

    const auto session = mUserSettings->getSession(did);

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

            mUserSettings->saveSession(*session->mBsky->getSession());
            mUserSettings->syncLater();
        },
        [this, did](const QString& msg){
            qWarning() << "Session refresh failed:" << msg;

            if (did == mSkywalker->getUserDid())
                emit activeSessionExpired(msg);
            else
                deleteSession(did);
        });

    const QString userDid = mSkywalker->getUserDid();

    if (did == userDid || mUserSettings->getNotificationsForAllAccounts(userDid))
        startNotificationRefreshTimers(did, initialDelayCount);
}

void SessionManager::stopRefreshTimers(const QString& did)
{
    qDebug() << "Stop refresh timers:" << did;
    auto* session = getSession(did);

    if (!session)
        return;

    session->mBsky->stopAutoRefresh();
    stopNotificationRefreshTimers(did);
}

void SessionManager::startNotificationRefreshTimers(const QString& did, int initialDelayCount)
{
    auto* session = getSession(did);

    if (!session)
        return;

    refreshNotificationCount(did);
    session->mRefreshNotificationInitialDelayTimer.start(initialDelayCount * NOTIFICATION_REFRESH_DELAY);
}

void SessionManager::stopNotificationRefreshTimers(const QString& did)
{
    auto* session = getSession(did);

    if (!session)
        return;

    session->mRefreshNotificationInitialDelayTimer.stop();
    session->mRefreshNotificationTimer.stop();
}

void SessionManager::enableNotificationsNonActiveUsers()
{
    int refreshDelayCount = 1;

    for (const auto& [did, _] : mDidSessionMap)
    {
        if (did != mSkywalker->getUserDid())
            startNotificationRefreshTimers(did, refreshDelayCount++);
    }
}

void SessionManager::disableNotificatiosNonActiveUsers()
{
    for (const auto& [did, _] : mDidSessionMap)
    {
        if (did != mSkywalker->getUserDid())
        {
            stopNotificationRefreshTimers(did);
            setUnreadNotificationCount(did, 0);
        }
    }
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

    if (session->mUnreadNotificationCount != unread)
    {
        session->mUnreadNotificationCount = unread;

        if (session->mNonActiveUser)
            session->mNonActiveUser->setUnreadNotificationCount(unread);
        else if (did == mSkywalker->getUserDid())
            emit activeUserUnreadNotificationCountChanged();

        const int totalUnread = getTotalUnreadNotificationCount();
        emit totalUnreadNotificationCountChanged(totalUnread);
    }
}

int SessionManager::getUnreadNotificationCount(const QString& did) const
{
    auto* session = getSession(did);
    return session ? session->mUnreadNotificationCount : 0;
}

int SessionManager::getTotalUnreadNotificationCount() const
{
    int unread = 0;

    for (const auto& [_, session] : mDidSessionMap)
        unread += session->mUnreadNotificationCount;

    return unread;
}

int SessionManager::getActiveUserUnreadNotificationCount() const
{
    const auto did = mSkywalker->getUserDid();
    return getUnreadNotificationCount(did);
}

const NonActiveUser::List& SessionManager::getNonActiveNotifications() const
{
    static const NonActiveUser::List EMPTY_LIST;

    const auto did = mSkywalker->getUserDid();

    if (!mUserSettings->getNotificationsForAllAccounts(did))
        return EMPTY_LIST;

    return mNonActiveUsers;
}

NonActiveUser* SessionManager::getNonActiveUserUnexpired(const QString& did) const
{
    auto* session = getSession(did);
    return session ? session->mNonActiveUser.get() : nullptr;
}

ATProto::Client* SessionManager::getActiveUserBskyClient() const
{
    return mSkywalker->getBskyClient();
}

ATProto::Client::SharedPtr SessionManager::getBskyClientFor(const QString& did) const
{
    auto* session = getSession(did);
    return session ? session->mSharedBsky : nullptr;
}

Skywalker* SessionManager::getSkywalker()
{
    return mSkywalker;
}

void SessionManager::pause()
{
    const QString userDid = mSkywalker->getUserDid();

    for (const auto& [did, session] : mDidSessionMap)
    {
        if (session)
        {
            if (did == userDid || mUserSettings->getNotificationsForAllAccounts(userDid))
                mUserSettings->setOfflineUnread(did, session->mUnreadNotificationCount);
        }
    }

    saveTokens();
}

void SessionManager::resume()
{
    updateTokens();
}

}
