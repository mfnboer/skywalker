// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"
#include "author_cache.h"
#include <QLoggingCategory>
#include <QSettings>

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto SESSION_REFRESH_INTERVAL = 299s;
static constexpr auto NOTIFICATION_REFRESH_INTERVAL = 29s;
static constexpr int TIMELINE_ADD_PAGE_SIZE = 50;
static constexpr int TIMELINE_PREPEND_PAGE_SIZE = 20;
static constexpr int TIMELINE_SYNC_PAGE_SIZE = 100;
static constexpr int TIMELINE_DELETE_SIZE = 100; // must not be smaller than add/sync
static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 25;
static constexpr int AUTHOR_ADD_PAGE_SIZE = 100; // Most posts are replies and are filtered

Skywalker::Skywalker(QObject* parent) :
    QObject(parent),
    mTimelineModel(mUserDid, mUserFollows, this)
{
    connect(&mRefreshTimer, &QTimer::timeout, this, [this]{ refreshSession(); });
    connect(&mRefreshNotificationTimer, &QTimer::timeout, this, [this]{ refreshNotificationCount(); });
    AuthorCache::instance().addProfileStore(&mUserFollows);
}

Skywalker::~Skywalker()
{
    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
}

void Skywalker::login(const QString user, QString password, const QString host)
{
    qInfo() << "Login:" << user << "host:" << host;
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));
    mBsky->createSession(user, password,
        [this, user, host]{
            qInfo() << "Login" << user << "succeeded";
            saveSession(host, *mBsky->getSession());
            mUserDid = mBsky->getSession()->mDid;
            emit loginOk();
            startRefreshTimers();
        },
        [this, user](const QString& error){
            qInfo() << "Login" << user << "failed:" << error;
            emit loginFailed(error);
        });
}

void Skywalker::resumeSession()
{
    qInfo() << "Resume session";
    QString host;
    ATProto::ComATProtoServer::Session session;

    if (!getSession(host, session))
    {
        qWarning() << "No saved session";
        emit resumeSessionFailed();
        return;
    }

    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->resumeSession(session,
        [this, host] {
            qInfo() << "Session resumed";
            saveSession(host, *mBsky->getSession());
            mUserDid = mBsky->getSession()->mDid;
            emit resumeSessionOk();
            refreshSession();
            startRefreshTimers();
        },
        [this](const QString& error){
            qInfo() << "Session could not be resumed:" << error;
            emit resumeSessionFailed();
        });
}

void Skywalker::startRefreshTimers()
{
    qInfo() << "Refresh timers started";
    mRefreshTimer.start(SESSION_REFRESH_INTERVAL);
    refreshNotificationCount();
    mRefreshNotificationTimer.start(NOTIFICATION_REFRESH_INTERVAL);
}

void Skywalker::stopRefreshTimers()
{
    qInfo() << "Refresh timers stopped";
    mRefreshTimer.stop();
    mRefreshNotificationTimer.stop();
}

void Skywalker::refreshSession()
{
    Q_ASSERT(mBsky);
    qDebug() << "Refresh session";

    const auto* session = mBsky->getSession();
    if (!session)
    {
        qWarning() << "No session to refresh.";
        stopRefreshTimers();
        return;
    }

    mBsky->refreshSession(*session,
        [this]{
            qDebug() << "Session refreshed";
            saveSession(mBsky->getHost(), *mBsky->getSession());
        },
        [this](const QString& error){
            qDebug() << "Session could not be refreshed:" << error;
            emit sessionExpired(error);
            stopRefreshTimers();
        });
}

void Skywalker::refreshNotificationCount()
{
    Q_ASSERT(mBsky);
    qDebug() << "Refresh notification count";

    mBsky->getUnreadNotificationCount({},
        [this](int unread){
            qDebug() << "Unread notification count:" << unread;
            setUnreadNotificationCount(unread);
        },
        [this](const QString& error){
            qWarning() << "Failed to get unread notification count";
        });
}

void Skywalker::getUserProfileAndFollows()
{
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qInfo() << "Get user profile, handle:" << session->mHandle << "did:" << session->mDid;

    // Get profile and follows in one go. We do not need detailed profile data.
    mBsky->getFollows(session->mDid, 100, {},
        [this](auto follows){
            for (auto& profile : follows->mFollows)
                mUserFollows.add(BasicProfile(*profile));

            const auto& nextCursor = follows->mCursor;
            if (!nextCursor->isEmpty())
                getUserProfileAndFollowsNextPage(*nextCursor);
            else
                signalGetUserProfileOk(*follows->mSubject);
        },
        [this](const QString& error){
            qWarning() << error;
            mUserFollows.clear();
            emit getUserProfileFailed();
        });
}

void Skywalker::getUserProfileAndFollowsNextPage(const QString& cursor, int maxPages)
{   
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qInfo() << "Get user profile next page:" << cursor << ",handle:" << session->mHandle <<
            ",did:" << session->mDid << ",max pages:" << maxPages;

    mBsky->getFollows(session->mDid, 100, cursor,
        [this, maxPages](auto follows){
            for (auto& profile : follows->mFollows)
                mUserFollows.add(BasicProfile(*profile));

            const auto& nextCursor = follows->mCursor;
            if (nextCursor->isEmpty())
            {
                signalGetUserProfileOk(*follows->mSubject);
                return;
            }

            if (maxPages > 0)
                getUserProfileAndFollowsNextPage(*nextCursor, maxPages - 1);
            else
                qWarning() << "Max pages reached!";
        },
        [this](const QString& error){
            qWarning() << error;
            mUserFollows.clear();
            emit getUserProfileFailed();
        });
}

void Skywalker::signalGetUserProfileOk(const ATProto::AppBskyActor::ProfileView& user)
{
    qInfo() << "Got user:" << user.mHandle << "#follows:" << mUserFollows.size();
    AuthorCache::instance().setUser(BasicProfile(user));
    const auto avatar = user.mAvatar ? *user.mAvatar : QString();
    setAvatarUrl(avatar);
    emit getUserProfileOK();
}

void Skywalker::syncTimeline(int maxPages)
{
    const auto timestamp = getSyncTimestamp();

    if (!timestamp.isValid())
    {
        qInfo() << "No timestamp saved";
        getTimeline(TIMELINE_ADD_PAGE_SIZE);
        emit timelineSyncOK(-1);
        return;
    }

    disableDebugLogging(); // sync can cause a lot of logging
    syncTimeline(timestamp, maxPages);
}

void Skywalker::syncTimeline(QDateTime tillTimestamp, int maxPages, const QString& cursor)
{
    Q_ASSERT(mBsky);
    Q_ASSERT(tillTimestamp.isValid());
    qInfo() << "Sync timeline:" << tillTimestamp << "max pages:" << maxPages;

    if (mGetTimelineInProgress)
    {
        qInfo() << "Get timeline still in progress";
        return;
    }

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_SYNC_PAGE_SIZE, makeOptionalCursor(cursor),
        [this, tillTimestamp, maxPages, cursor](auto feed){
            mTimelineModel.addFeed(std::move(feed));
            setGetTimelineInProgress(false);
            const auto lastTimestamp = mTimelineModel.lastTimestamp();

            if (lastTimestamp.isNull())
            {
                restoreDebugLogging();
                qWarning() << "Feed is empty";
                emit timelineSyncFailed();
                return;
            }

            if (lastTimestamp < tillTimestamp)
            {
                restoreDebugLogging();
                const auto index = mTimelineModel.findTimestamp(tillTimestamp);
                qDebug() << "Timeline synced, last timestamp:" << lastTimestamp << "index:"
                        << index << ",feed size:" << mTimelineModel.rowCount()
                        << ",pages left:" << maxPages;

                Q_ASSERT(index >= 0);
                const auto& post = mTimelineModel.getPost(index);
                qDebug() << post.getTimelineTimestamp() << post.getText();

                emit timelineSyncOK(index);
                return;
            }

            if (maxPages == 1)
            {
                restoreDebugLogging();
                qDebug() << "Max pages loaded, failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;
                emit timelineSyncOK(mTimelineModel.rowCount() - 1);
                return;
            }

            const QString& newCursor = mTimelineModel.getLastCursor();
            if (newCursor.isEmpty())
            {
                restoreDebugLogging();
                qDebug() << "Last page reached, no more cursor";
                emit timelineSyncOK(mTimelineModel.rowCount() - 1);
                return;
            }

            if (newCursor == cursor)
            {
                restoreDebugLogging();
                qWarning() << "New cursor:" << newCursor << "is same as previous:" << cursor;
                qDebug() << "Failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;
                emit timelineSyncOK(mTimelineModel.rowCount() - 1);
            }

            qInfo() << "Last timestamp:" << lastTimestamp;
            syncTimeline(tillTimestamp, maxPages - 1, newCursor);
        },
        [this](const QString& error){
            restoreDebugLogging();
            qWarning() << "syncTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
            emit timelineSyncFailed();
        }
        );
}

void Skywalker::getTimeline(int limit, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline:" << cursor;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    setGetTimelineInProgress(true);
    mBsky->getTimeline(limit, makeOptionalCursor(cursor),
       [this, cursor](auto feed){
            if (cursor.isEmpty())
                mTimelineModel.setFeed(std::move(feed));
            else
                mTimelineModel.addFeed(std::move(feed));

            setGetTimelineInProgress(false);
       },
       [this](const QString& error){
            qInfo() << "getTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        }
    );
}

void Skywalker::getTimelinePrepend(int autoGapFill)
{
    Q_ASSERT(mBsky);
    qInfo() << "Get timeline prepend";

    if (mGetTimelineInProgress)
    {
        qInfo() << "Get timeline still in progress";
        return;
    }

    if (mTimelineModel.rowCount() >= PostFeedModel::MAX_TIMELINE_SIZE)
    {
        qInfo() << "Timeline is full:" << mTimelineModel.rowCount();
        return;
    }

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_PREPEND_PAGE_SIZE, {},
        [this, autoGapFill](auto feed){
            const int gapId = mTimelineModel.prependFeed(std::move(feed));
            setGetTimelineInProgress(false);

            if (gapId > 0)
            {
                if (autoGapFill > 0)
                    getTimelineForGap(gapId, autoGapFill - 1);
                else
                    qDebug() << "Gap created, no auto gap fill";
            }
        },
        [this](const QString& error){
            qDebug() << "getTimeline FAILED:" << error;
            setGetTimelineInProgress(false);
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        }
        );
}

void Skywalker::getTimelineForGap(int gapId, int autoGapFill)
{
    Q_ASSERT(mBsky);
    qInfo() << "Get timeline for gap:" << gapId;

    if (mGetTimelineInProgress)
    {
        qInfo() << "Get timeline still in progress";
        return;
    }

    const Post* post = mTimelineModel.getGapPlaceHolder(gapId);
    if (!post || !post->isGap())
    {
        qWarning() << "NO GAP:" << gapId;
        return;
    }

    std::optional<QString> cur = post->getGapCursor();
    if (!cur || cur->isEmpty())
    {
        qWarning() << "NO CURSOR FOR GAP:" << gapId;
        return;
    }

    qInfo() << "Set gap cursor:" << *cur;

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_ADD_PAGE_SIZE, cur,
        [this, gapId, autoGapFill](auto feed){
            const int newGapId = mTimelineModel.gapFillFeed(std::move(feed), gapId);
            setGetTimelineInProgress(false);

            if (newGapId > 0)
            {
                if (autoGapFill > 0)
                    getTimelineForGap(newGapId, autoGapFill - 1);
                else
                    qInfo() << "Gap created, no auto gap fill";
            }
        },
        [this](const QString& error){
            qInfo() << "getTimelineForGap FAILED:" << error;
            setGetTimelineInProgress(false);
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        }
        );
}

void Skywalker::getTimelineNextPage()
{
    const QString& cursor = mTimelineModel.getLastCursor();
    if (cursor.isEmpty())
    {
        qInfo() << "Last page reached, no more cursor";
        return;
    }

    if (mTimelineModel.rowCount() >= PostFeedModel::MAX_TIMELINE_SIZE)
        mTimelineModel.removeHeadPosts(TIMELINE_ADD_PAGE_SIZE);

    getTimeline(TIMELINE_ADD_PAGE_SIZE, cursor);
}

void Skywalker::setGetTimelineInProgress(bool inProgress)
{
    mGetTimelineInProgress = inProgress;
    emit getTimeLineInProgressChanged();
}

void Skywalker::setGetPostThreadInProgress(bool inProgress)
{
    mGetPostThreadInProgress = inProgress;
}

void Skywalker::setGetNotificationsInProgress(bool inProgress)
{
    mGetNotificationsInProgress = inProgress;
    emit getNotificationsInProgressChanged();
}

void Skywalker::setGetAuthorFeedInProgress(bool inProgress)
{
    mGetAuthorFeedInProgress = inProgress;
    emit getAuthorFeedInProgressChanged();
}

void Skywalker::setAvatarUrl(const QString& avatarUrl)
{
    mAvatarUrl = avatarUrl;
    emit avatarUrlChanged();
}

void Skywalker::setUnreadNotificationCount(int unread)
{
    mUnreadNotificationCount = unread;
    emit unreadNotificationCountChanged();
}

// NOTE: indices can be -1 if the UI cannot determine the index
void Skywalker::timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex)
{
    if (lastVisibleIndex > -1)
    {
        if (firstVisibleIndex > -1)
            saveSyncTimestamp((lastVisibleIndex + firstVisibleIndex) / 2);
        else
            saveSyncTimestamp(lastVisibleIndex);
    }

    if (lastVisibleIndex > -1 && mTimelineModel.rowCount() - lastVisibleIndex > 2 * TIMELINE_DELETE_SIZE)
        mTimelineModel.removeTailPosts(TIMELINE_DELETE_SIZE);

    if (lastVisibleIndex > mTimelineModel.rowCount() - 5 && !mGetTimelineInProgress)
        getTimelineNextPage();
}

void Skywalker::getPostThread(const QString& uri)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get post thread:" << uri;

    if (mGetPostThreadInProgress)
    {
        qDebug() << "Get post thread still in progress";
        return;
    }

    setGetPostThreadInProgress(true);
    mBsky->getPostThread(uri, {}, {},
        [this](auto thread){
            setGetPostThreadInProgress(false);
            auto model = std::make_unique<PostThreadModel>(mUserDid, mUserFollows, this);
            int postEntryIndex = model->setPostThread(std::move(thread));

            if (postEntryIndex < 0)
            {
                qDebug() << "No thread posts";
                emit statusMessage("Could not create post thread", QEnums::STATUS_LEVEL_ERROR);
                return;
            }

            int id = mPostThreadModels.put(std::move(model));
            emit postThreadOk(id, postEntryIndex);
        },
        [this](const QString& error){
            setGetPostThreadInProgress(false);
            qDebug() << "getPostThread FAILED:" << error;
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

std::optional<QString> Skywalker::makeOptionalCursor(const QString& cursor) const
{
    std::optional<QString> optionalCursor;
    if (!cursor.isEmpty())
        optionalCursor = cursor;

    return optionalCursor;
}

const PostThreadModel* Skywalker::getPostThreadModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mPostThreadModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removePostThreadModel(int id)
{
    qDebug() << "Remove model:" << id;
    mPostThreadModels.remove(id);
}

void Skywalker::updatePostIndexTimestamps()
{
    makeLocalModelChange([](auto* model){ model->updatePostIndexTimestamps(); });
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalPostModelChanges*)>& update)
{
    // Apply change to all active models. When a model gets refreshed (after clear)
    // or deleted, then the local changes will disapper.

    update(&mTimelineModel);
    update(&mNotificationListModel);

    for (auto& [_, model] : mPostThreadModels.items())
        update(model.get());
}

void Skywalker::getNotifications(int limit, bool updateSeen, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get notifications:" << cursor;

    if (mGetNotificationsInProgress)
    {
        qDebug() << "Get notifications still in progress";
        return;
    }

    setGetNotificationsInProgress(true);
    mBsky->listNotifications(limit, makeOptionalCursor(cursor), {},
        [this, cursor](auto list){
            const bool clearFirst = cursor.isEmpty();
            mNotificationListModel.addNotifications(std::move(list), *mBsky, clearFirst);
            setGetNotificationsInProgress(false);
        },
        [this](const QString& error){
            qDebug() << "getNotifications FAILED:" << error;
            setGetNotificationsInProgress(false);
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        },
        updateSeen);

    if (updateSeen)
        setUnreadNotificationCount(0);
}

void Skywalker::getNotificationsNextPage()
{
    const QString& cursor = mNotificationListModel.getCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, cursor);
}

void Skywalker::getDetailedProfile(const QString& author)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get detailed profile:" << author;

    mBsky->getProfile(author,
        [this](auto profile){
            auto shared = ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr(profile.release());
            emit getDetailedProfileOK(DetailedProfile(shared));
        },
        [this](const QString& error){
            qDebug() << "getDetailedProfile failed:" << error;
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

Q_INVOKABLE void Skywalker::getAuthorFeed(int id, int limit, int maxPages, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author feed model:" << id << "cursor:" << cursor << "max pages:" << maxPages;

    if (mGetAuthorFeedInProgress)
    {
        qDebug() << "Get author feed still in progress";
        return;
    }

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const auto& author = (*model)->getAuthor();
    qDebug() << "Get author feed:" << author;

    setGetAuthorFeedInProgress(true);
    mBsky->getAuthorFeed(author, limit, makeOptionalCursor(cursor),
        [this, author, id, model, maxPages, cursor](auto feed){
            setGetAuthorFeedInProgress(false);
            bool feedAdded = cursor.isEmpty() ?
                    (*model)->setFeed(std::move(feed)) :
                    (*model)->addFeed(std::move(feed));

            if (!feedAdded)
                getAuthorFeedNextPage(id, maxPages - 1);
        },
        [this](const QString& error){
            setGetAuthorFeedInProgress(false);
            qDebug() << "getAuthorFeed failed:" << error;
            emit statusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorFeedNextPage(int id, int maxPages)
{
    qDebug() << "Get author feed next page, model:" << id << "max pages:" << maxPages;

    if (mGetAuthorFeedInProgress)
    {
        qDebug() << "Get author feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        // Protection against infinite loop.
        qWarning() << "Maximum pages reached!";
        return;
    }

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    auto* authorFeedModel = (*model).get();
    const auto& cursor = authorFeedModel->getCursorNextPage();

    if (cursor.isEmpty())
    {
        qDebug() << "End of page reached.";
        return;
    }

    getAuthorFeed(id, AUTHOR_ADD_PAGE_SIZE, maxPages, cursor);
}

int Skywalker::createAuthorFeedModel(const QString& author)
{
    auto model = std::make_unique<AuthorFeedModel>(author, mUserDid, mUserFollows, this);
    const int id = mAuthorFeedModels.put(std::move(model));
    return id;
}

const AuthorFeedModel* Skywalker::getAuthorFeedModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mAuthorFeedModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeAuthorFeedModel(int id)
{
    qDebug() << "Remove model:" << id;
    mAuthorFeedModels.remove(id);
}

void Skywalker::saveSession(const QString& host, const ATProto::ComATProtoServer::Session& session)
{
    // TODO: secure storage
    mSettings.setValue("host", host);
    mSettings.setValue("did", session.mDid);
    mSettings.setValue("access", session.mAccessJwt);
    mSettings.setValue("refresh", session.mRefreshJwt);
}

bool Skywalker::getSession(QString& host, ATProto::ComATProtoServer::Session& session)
{
    host = mSettings.value("host").toString();
    session.mDid = mSettings.value("did").toString();
    session.mAccessJwt = mSettings.value("access").toString();
    session.mRefreshJwt = mSettings.value("refresh").toString();

    return !(host.isEmpty() || session.mDid.isEmpty() || session.mAccessJwt.isEmpty() || session.mRefreshJwt.isEmpty());
}

void Skywalker::saveSyncTimestamp(int postIndex)
{
    if (postIndex < 0 || postIndex >= mTimelineModel.rowCount())
    {
        qWarning() << "Invalid index:" << postIndex << "size:" << mTimelineModel.rowCount();
        return;
    }

    const auto& post = mTimelineModel.getPost(postIndex);
    mSettings.setValue("syncTimestamp", post.getTimelineTimestamp());
}

QDateTime Skywalker::getSyncTimestamp() const
{
    return mSettings.value("syncTimestamp").toDateTime();
}

void Skywalker::disableDebugLogging()
{
    mDebugLogging = QLoggingCategory::defaultCategory()->isDebugEnabled();
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, false);
}

void Skywalker::restoreDebugLogging()
{
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, mDebugLogging);
}

}
