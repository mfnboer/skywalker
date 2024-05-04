// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"
#include "author_cache.h"
#include "definitions.h"
#include "file_utils.h"
#include "jni_callback.h"
#include "offline_message_checker.h"
#include "photo_picker.h"
#include "shared_image_provider.h"
#include <atproto/lib/at_uri.h>
#include <QClipboard>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QSettings>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace Skywalker {

using namespace std::chrono_literals;

// There is a trade off: short timeout is fast updating timeline, long timeout
// allows for better reply thread construction as we receive more posts per update.
static constexpr auto TIMELINE_UPDATE_INTERVAL = 91s;

static constexpr auto SESSION_REFRESH_INTERVAL = 299s;
static constexpr auto NOTIFICATION_REFRESH_INTERVAL = 29s;
static constexpr int TIMELINE_ADD_PAGE_SIZE = 50;
static constexpr int TIMELINE_PREPEND_PAGE_SIZE = 20;
static constexpr int TIMELINE_SYNC_PAGE_SIZE = 100;
static constexpr int TIMELINE_DELETE_SIZE = 100; // must not be smaller than add/sync
static constexpr int FEED_ADD_PAGE_SIZE = 50;
static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 25;
static constexpr int AUTHOR_FEED_ADD_PAGE_SIZE = 100; // Most posts are replies and are filtered
static constexpr int AUTHOR_LIKES_ADD_PAGE_SIZE = 25;
static constexpr int AUTHOR_LIST_ADD_PAGE_SIZE = 50;
static constexpr int USER_HASHTAG_INDEX_SIZE = 100;
static constexpr int SEEN_HASHTAG_INDEX_SIZE = 500;

Skywalker::Skywalker(QObject* parent) :
    QObject(parent),
    mContentFilter(mUserPreferences),
    mBookmarks(this),
    mMutedWords(this),
    mNotificationListModel(mContentFilter, mBookmarks, mMutedWords, this),
    mUserHashtags(USER_HASHTAG_INDEX_SIZE),
    mSeenHashtags(SEEN_HASHTAG_INDEX_SIZE),
    mFavoriteFeeds(this),
    mUserSettings(this),
    mTimelineModel(HOME_FEED, mUserDid, mUserFollows, mMutedReposts, mContentFilter,
                   mBookmarks, mMutedWords, mSeenHashtags, mUserPreferences, mUserSettings, this)
{
    mBookmarks.setSkywalker(this);
    connect(&mBookmarks, &Bookmarks::sizeChanged, this, [this]{ mBookmarks.save(); });

    connect(&mRefreshTimer, &QTimer::timeout, this, [this]{ refreshSession(); });
    connect(&mRefreshNotificationTimer, &QTimer::timeout, this, [this]{ refreshNotificationCount(); });

    connect(&mTimelineUpdateTimer, &QTimer::timeout, this,
            [this]{
                getTimelinePrepend(2);
                updatePostIndexTimestamps();
            });

    AuthorCache::instance().addProfileStore(&mUserFollows);
    OffLineMessageChecker::createNotificationChannels();

    auto& jniCallbackListener = JNICallbackListener::getInstance();
    connect(&jniCallbackListener, &JNICallbackListener::sharedTextReceived, this,
            [this](const QString& text){ emit sharedTextReceived(text); });
    connect(&jniCallbackListener, &JNICallbackListener::sharedImageReceived, this,
            [this](const QString& contentUri, const QString& text){ shareImage(contentUri, text); });
    connect(&jniCallbackListener, &JNICallbackListener::showNotifications, this,
            [this]{ emit showNotifications(); });

    auto* app = (QGuiApplication*)QGuiApplication::instance();
    Q_ASSERT(app);

    if (app)
    {
        connect(app, &QGuiApplication::applicationStateChanged, this,
                [this](Qt::ApplicationState state){ handleAppStateChange(state); });
    }
    else
    {
        qWarning() << "Failed to get app instance!";
    }
}

Skywalker::~Skywalker()
{
    saveHashtags();
    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
    Q_ASSERT(mSearchPostFeedModels.empty());
    Q_ASSERT(mAuthorListModels.empty());
    Q_ASSERT(mListListModels.empty());
    Q_ASSERT(mFeedListModels.empty());
    Q_ASSERT(mContentGroupListModels.empty());
}

// NOTE: user can be handle or DID
void Skywalker::login(const QString user, QString password, const QString host, const QString authFactorToken)
{
    qDebug() << "Login:" << user << "host:" << host;
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));
    mBsky->createSession(user, password, makeOptionalString(authFactorToken),
        [this, host, user]{
            qDebug() << "Login" << user << "succeeded";
            const auto* session = mBsky->getSession();
            updateUser(session->mDid, host);
            saveSession(*session);
            emit loginOk();
            startRefreshTimers();
        },
        [this, host, user, password](const QString& error, const QString& msg){
            qDebug() << "Login" << user << "failed:" << error << " - " << msg;
            mUserSettings.setActiveUserDid({});
            emit loginFailed(error, msg, host, user, password);
        });
}

bool Skywalker::resumeSession(bool retry)
{
    qDebug() << "Resume session, retry:" << retry;
    QString host;
    ATProto::ComATProtoServer::Session session;

    if (!getSavedSession(host, session))
    {
        qDebug() << "No saved session";
        return false;
    }

    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->resumeSession(session,
        [this, retry] {
            qInfo() << "Session resumed";
            saveSession(*mBsky->getSession());
            mUserDid = mBsky->getSession()->mDid;
            emit resumeSessionOk();

            if (!retry)
                refreshSession();

            startRefreshTimers();
        },
        [this, retry, session](const QString& error, const QString& msg){
            qInfo() << "Session could not be resumed:" << error << " - " << msg;

            if (!retry && error == ATProto::ATProtoErrorMsg::EXPIRED_TOKEN)
            {
                mBsky->setSession(std::make_unique<ATProto::ComATProtoServer::Session>(session));
                mBsky->refreshSession(
                    [this]{
                        qDebug() << "Session refreshed";
                        saveSession(*mBsky->getSession());
                        resumeSession(true);
                    },
                    [this](const QString& error, const QString& msg){
                        qDebug() << "Session could not be refreshed:" << error << " - " << msg;
                        emit resumeSessionFailed(msg);
                    });
            }
            else
            {
                emit resumeSessionFailed(msg);
            }
        });

    return true;
}

void Skywalker::deleteSession()
{
    Q_ASSERT(mBsky);
    qDebug() << "Delete session";
    mBsky->deleteSession({}, {});
}

void Skywalker::switchUser(const QString& did)
{
    qDebug() << "Switch to user:" << did;
    mUserDid = did;
    mUserSettings.setActiveUserDid(did);
}

void Skywalker::startTimelineAutoUpdate()
{
    qDebug() << "Start timeline auto update";
    mTimelineUpdateTimer.start(TIMELINE_UPDATE_INTERVAL);
}

void Skywalker::stopTimelineAutoUpdate()
{
    qDebug() << "Stop timeline auto update";
    mTimelineUpdateTimer.stop();
}

void Skywalker::startRefreshTimers()
{
    qDebug() << "Refresh timers started";
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

    mBsky->refreshSession(
        [this]{
            qDebug() << "Session refreshed";
            saveSession(*mBsky->getSession());
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "Session could not be refreshed:" << error << " - " << msg;
            emit sessionExpired(msg);
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
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get unread notification count:" << error << " - " << msg;
        });
}

void Skywalker::getUserProfileAndFollows()
{
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qDebug() << "Get user profile, handle:" << session->mHandle << "did:" << session->mDid;

    // Get profile and follows in one go. We do not need detailed profile data.
    mBsky->getFollows(session->mDid, 100, {},
        [this](auto follows){
            for (auto& profile : follows->mFollows)
                mUserFollows.add(BasicProfile(*profile));

            const auto& nextCursor = follows->mCursor;
            if (!nextCursor->isEmpty())
                getUserProfileAndFollowsNextPage(*nextCursor);
            else
                signalGetUserProfileOk(std::move(follows->mSubject));
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            mUserFollows.clear();
            emit getUserProfileFailed(msg);
        });
}

void Skywalker::getUserProfileAndFollowsNextPage(const QString& cursor, int maxPages)
{   
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qDebug() << "Get user profile next page:" << cursor << ", handle:" << session->mHandle <<
            ", did:" << session->mDid << ", max pages:" << maxPages;

    mBsky->getFollows(session->mDid, 100, cursor,
        [this, maxPages](auto follows){
            for (auto& profile : follows->mFollows)
                mUserFollows.add(BasicProfile(*profile));

            const auto& nextCursor = follows->mCursor;

            if (nextCursor->isEmpty())
            {
                signalGetUserProfileOk(std::move(follows->mSubject));
            }
            else if (maxPages > 0)
            {
                getUserProfileAndFollowsNextPage(*nextCursor, maxPages - 1);
            }
            else
            {
                qWarning() << "Max pages reached!";
                signalGetUserProfileOk(std::move(follows->mSubject));
            }
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            mUserFollows.clear();
            emit getUserProfileFailed(msg);
        });
}

void Skywalker::signalGetUserProfileOk(ATProto::AppBskyActor::ProfileView::Ptr user)
{
    Q_ASSERT(mUserDid == user->mDid);
    qDebug() << "Got user:" << user->mHandle << "#follows:" << mUserFollows.size();
    AuthorCache::instance().setUser(BasicProfile(*user));
    mUserSettings.saveDisplayName(mUserDid, user->mDisplayName.value_or(""));
    const auto avatar = user->mAvatar ? *user->mAvatar : QString();
    mUserSettings.saveAvatar(mUserDid, avatar);
    mLoggedOutVisibility = ATProto::ProfileMaster::getLoggedOutVisibility(*user);
    auto sharedUser = ATProto::AppBskyActor::ProfileView::SharedPtr(user.release());
    mUserProfile = Profile(sharedUser);

    emit avatarUrlChanged();
    emit getUserProfileOK();
}

void Skywalker::getUserPreferences()
{
    Q_ASSERT(mBsky);
    qDebug() << "Get user preferences";

    mBsky->getPreferences(
        [this](auto prefs){
            mUserPreferences = prefs;
            updateFavoriteFeeds();
            loadMutedReposts();
            initLabelers();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            emit getUserPreferencesFailed();
        });
}

void Skywalker::updateFavoriteFeeds()
{
    qDebug() << "Update favorite feeds";
    const auto& savedFeedsPref = mUserPreferences.getSavedFeedsPref();
    mFavoriteFeeds.reset(savedFeedsPref);
}

void Skywalker::saveFavoriteFeeds()
{
    qDebug() << "Save favorite feeds";
    auto prefs = mUserPreferences;
    mFavoriteFeeds.saveTo(prefs);
    saveUserPreferences(prefs);
}

void Skywalker::loadBookmarks()
{
    mBookmarks.load();
}

void Skywalker::loadMutedWords()
{
    mMutedWords.load(mUserPreferences);

    if (mMutedWords.legacyLoad(&mUserSettings))
    {
        qDebug() << "Migrate locally stored muted words to bsky";
        saveMutedWords([this, did=mUserDid]{ mUserSettings.removeMutedWords(did); });
    }
    else
    {
        // There were no muted words stored locally, but the user setting keys may
        // be stored. Remove those.
        mUserSettings.removeMutedWords(mUserDid);
    }
}

void Skywalker::saveMutedWords(std::function<void()> okCb)
{
    if (!mMutedWords.isDirty())
        return;

    qDebug() << "Save muted words";
    auto prefs = mUserPreferences;
    mMutedWords.save(prefs);
    saveUserPreferences(prefs, okCb);
}

void Skywalker::loadHashtags()
{
    qDebug() << "Load hashtags";

    mUserHashtags.clear();
    mUserHashtags.insert(mUserSettings.getUserHashtags(mUserDid));
    mUserHashtags.setDirty(false);

    mSeenHashtags.clear();
    mSeenHashtags.insert(mUserSettings.getSeenHashtags());
    mSeenHashtags.setDirty(false);
}

void Skywalker::saveHashtags()
{
    qDebug() << "Save hashtags";

    if (mUserHashtags.isDirty())
    {
        mUserSettings.setUserHashtags(mUserDid, mUserHashtags.getAllHashtags());
        mUserHashtags.setDirty(false);
    }

    if (mSeenHashtags.isDirty())
    {
        mUserSettings.setSeenHashtags(mSeenHashtags.getAllHashtags());
        mSeenHashtags.setDirty(false);
    }
}

void Skywalker::saveUserPreferences(const ATProto::UserPreferences& prefs, std::function<void()> okCb)
{
    Q_ASSERT(mBsky);
    qDebug() << "Save user preferences";

    mBsky->putPreferences(prefs,
        [this, prefs, okCb]{
            qDebug() << "saveUserPreferences ok";
            mUserPreferences = prefs;
            initLabelers();

            if (okCb)
                okCb();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "saveUserPreferences failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::loadMutedReposts(int maxPages, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Load muted reposts, maxPages:" << maxPages << "cursor:" << cursor;

    const QString uri = mUserSettings.getMutedRepostsListUri(mUserDid);
    mMutedReposts.setListUri(uri);

    if (maxPages <= 0)
    {
        qWarning() << "Max pages reached";

        emit statusMessage(tr("Too many muted reposts!"), QEnums::STATUS_LEVEL_ERROR);

        // Either their are too many muted reposts, or the cursor got in a loop.
        // We signal OK as there is no way out of this situation without starting
        // up the app.
        emit getUserPreferencesOK();
        return;
    }

    mBsky->getList(uri, 100, makeOptionalString(cursor),
        [this, maxPages](auto output){
            mMutedReposts.setListCreated(true);

            for (const auto& item : output->mItems)
            {
                const BasicProfile profile(item->mSubject.get());
                mMutedReposts.add(profile, item->mUri);
            }

            if (output->mCursor)
                loadMutedReposts(maxPages - 1, *output->mCursor);
            else
                emit getUserPreferencesOK();
        },
        [this](const QString& error, const QString& msg){
            mMutedReposts.setListCreated(false);

            if (ATProto::Client::isListNotFoundError(error, msg))
            {
                qDebug() << "No muted reposts list:" << error << " - " << msg;
                emit getUserPreferencesOK();
            }
            else
            {
                qWarning() << "loadMutedReposts failed:" << error << " - " << msg;
                emit getUserPreferencesFailed();
            }
        });
}

void Skywalker::initLabelers()
{
    Q_ASSERT(mBsky);
    mBsky->setLabelerDids(mContentFilter.getSubscribedLabelerDids());
}

void Skywalker::dataMigration()
{
    migrateDraftPosts();
}

void Skywalker::syncTimeline(int maxPages)
{
    const auto timestamp = getSyncTimestamp();

    if (!timestamp.isValid() || !mUserSettings.getRewindToLastSeenPost(mUserDid))
    {
        qDebug() << "Do not rewind timeline";
        getTimeline(TIMELINE_ADD_PAGE_SIZE);
        finishTimelineSync(-1);
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
    mBsky->getTimeline(TIMELINE_SYNC_PAGE_SIZE, makeOptionalString(cursor),
        [this, tillTimestamp, maxPages, cursor](auto feed){
            mTimelineModel.addFeed(std::move(feed));
            setGetTimelineInProgress(false);
            const auto lastTimestamp = mTimelineModel.lastTimestamp();

            if (lastTimestamp.isNull())
            {
                restoreDebugLogging();
                qWarning() << "Feed is empty";
                finishTimelineSyncFailed();
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

                finishTimelineSync(index);
                return;
            }

            if (maxPages == 1)
            {
                restoreDebugLogging();
                qDebug() << "Max pages loaded, failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;
                finishTimelineSync(mTimelineModel.rowCount() - 1);
                return;
            }

            const QString& newCursor = mTimelineModel.getLastCursor();
            if (newCursor.isEmpty())
            {
                restoreDebugLogging();
                qDebug() << "Last page reached, no more cursor";
                finishTimelineSync(mTimelineModel.rowCount() - 1);
                return;
            }

            if (newCursor == cursor)
            {
                restoreDebugLogging();
                qWarning() << "New cursor:" << newCursor << "is same as previous:" << cursor;
                qDebug() << "Failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;
                finishTimelineSync(mTimelineModel.rowCount() - 1);
            }

            qInfo() << "Last timestamp:" << lastTimestamp;
            syncTimeline(tillTimestamp, maxPages - 1, newCursor);
        },
        [this](const QString& error, const QString& msg){
            restoreDebugLogging();
            qWarning() << "syncTimeline FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
            finishTimelineSyncFailed();
        }
        );
}

void Skywalker::finishTimelineSync(int index)
{
    // Inform the GUI about the timeline sync.
    // This will show the timeline to the user.
    emit timelineSyncOK(index);
    OffLineMessageChecker::checkNoticationPermission();

    // Now we can handle pending intent (content share).
    // If there is any, then this will open the post composition page. This should
    // only been done when the startup sequence in the GUI is finished.
    JNICallbackListener::handlePendingIntent();
}

void Skywalker::finishTimelineSyncFailed()
{
    emit timelineSyncFailed();
    OffLineMessageChecker::checkNoticationPermission();
    JNICallbackListener::handlePendingIntent();
}

void Skywalker::getTimeline(int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline:" << cursor;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    setGetTimelineInProgress(true);
    mBsky->getTimeline(limit, makeOptionalString(cursor),
       [this, maxPages, minEntries, cursor](auto feed){
            setGetTimelineInProgress(false);
            int topPostIndex = -1;
            int addedPosts = 0;

            if (cursor.isEmpty())
            {
                topPostIndex = mTimelineModel.setFeed(std::move(feed));
                addedPosts = mTimelineModel.rowCount();
            }
            else
            {
                const int oldRowCount = mTimelineModel.rowCount();
                mTimelineModel.addFeed(std::move(feed));
                addedPosts = mTimelineModel.rowCount() - oldRowCount;
            }

            const int postsToAdd = minEntries - addedPosts;

            if (postsToAdd > 0)
                getTimelineNextPage(maxPages - 1, postsToAdd);

            if (topPostIndex >= 0)
                emit timelineRefreshed(topPostIndex);
       },
       [this](const QString& error, const QString& msg){
            qInfo() << "getTimeline FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
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

    setAutoUpdateTimelineInProgress(true);
    setGetTimelineInProgress(true);

    mBsky->getTimeline(TIMELINE_PREPEND_PAGE_SIZE, {},
        [this, autoGapFill](auto feed){
            const int gapId = mTimelineModel.prependFeed(std::move(feed));
            setGetTimelineInProgress(false);
            setAutoUpdateTimelineInProgress(false);

            if (gapId > 0)
            {
                if (autoGapFill > 0)
                    getTimelineForGap(gapId, autoGapFill - 1);
                else
                    qDebug() << "Gap created, no auto gap fill";
            }
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getTimeline FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            setAutoUpdateTimelineInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        }
        );
}

void Skywalker::getTimelineForGap(int gapId, int autoGapFill, bool userInitiated)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline for gap:" << gapId;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
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

    qDebug() << "Set gap cursor:" << *cur;

    setGetTimelineInProgress(true);
    mBsky->getTimeline(TIMELINE_ADD_PAGE_SIZE, cur,
        [this, gapId, autoGapFill, userInitiated](auto feed){
            const int newGapId = mTimelineModel.gapFillFeed(std::move(feed), gapId);
            setGetTimelineInProgress(false);

            if (userInitiated)
            {
                const int gapEndIndex = mTimelineModel.getLastInsertedRowIndex();
                emit gapFilled(gapEndIndex);
            }

            if (newGapId > 0)
            {
                if (autoGapFill > 0)
                    getTimelineForGap(newGapId, autoGapFill - 1, userInitiated);
                else
                    qDebug() << "Gap created, no auto gap fill";
            }
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "getTimelineForGap FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        }
        );
}

void Skywalker::getTimelineNextPage(int maxPages, int minEntries)
{
    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    const QString& cursor = mTimelineModel.getLastCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    if (mTimelineModel.rowCount() >= PostFeedModel::MAX_TIMELINE_SIZE)
        mTimelineModel.removeHeadPosts(TIMELINE_ADD_PAGE_SIZE);

    getTimeline(TIMELINE_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

void Skywalker::getFeed(int modelId, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get feed model:" << modelId << "cursor:" << cursor;

    if (mGetFeedInProgress)
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    const QString& feedUri = model->getGeneratorView().getUri();
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    setGetFeedInProgress(true);

    mBsky->getFeed(feedUri, limit, makeOptionalString(cursor), langs,
        [this, modelId, maxPages, minEntries, cursor](auto feed){
            setGetFeedInProgress(false);
            int addedPosts = 0;
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            if (cursor.isEmpty())
            {
                model->setFeed(std::move(feed));
                addedPosts = model->rowCount();
            }
            else
            {
                const int oldRowCount = model->rowCount();
                model->addFeed(std::move(feed));
                addedPosts = model->rowCount() - oldRowCount;
            }

            const int postsToAdd = minEntries - addedPosts;

            if (postsToAdd > 0)
                getFeedNextPage(modelId, maxPages - 1, postsToAdd);
        },
        [this](const QString& error, const QString& msg){
            qInfo() << "getFeed FAILED:" << error << " - " << msg;
            setGetFeedInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getFeedNextPage(int modelId, int maxPages, int minEntries)
{
    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    const QString& cursor = model->getLastCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getFeed(modelId, FEED_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

void Skywalker::getListFeed(int modelId, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get list feed model:" << modelId << "cursor:" << cursor;

    if (mGetFeedInProgress)
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    const QString& listUri = model->getListView().getUri();
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    setGetFeedInProgress(true);

    mBsky->getListFeed(listUri, limit, makeOptionalString(cursor), langs,
        [this, modelId, maxPages, minEntries, cursor](auto feed){
            setGetFeedInProgress(false);
            int addedPosts = 0;
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            if (cursor.isEmpty())
            {
                model->setFeed(std::move(feed));
                addedPosts = model->rowCount();
            }
            else
            {
                const int oldRowCount = model->rowCount();
                model->addFeed(std::move(feed));
                addedPosts = model->rowCount() - oldRowCount;
            }

            const int postsToAdd = minEntries - addedPosts;

            if (postsToAdd > 0)
                getListFeedNextPage(modelId, maxPages - 1, postsToAdd);
        },
        [this](const QString& error, const QString& msg){
            qInfo() << "getListFeed FAILED:" << error << " - " << msg;
            setGetFeedInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListFeedNextPage(int modelId, int maxPages, int minEntries)
{
    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    const QString& cursor = model->getLastCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getListFeed(modelId, FEED_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

void Skywalker::setAutoUpdateTimelineInProgress(bool inProgress)
{
    mAutoUpdateTimelineInProgress = inProgress;
    emit autoUpdateTimeLineInProgressChanged();
}

void Skywalker::setGetTimelineInProgress(bool inProgress)
{
    mGetTimelineInProgress = inProgress;
    emit getTimeLineInProgressChanged();
}

void Skywalker::setGetFeedInProgress(bool inProgress)
{
    mGetFeedInProgress = inProgress;
    emit getFeedInProgressChanged();
}

void Skywalker::setGetPostThreadInProgress(bool inProgress)
{
    mGetPostThreadInProgress = inProgress;
    emit getPostThreadInProgressChanged();
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

void Skywalker::setGetAuthorListInProgress(bool inProgress)
{
    mGetAuthorListInProgress = inProgress;
    emit getAuthorListInProgressChanged();
}

void Skywalker::setGetListListInProgress(bool inProgress)
{
    mGetListListInProgress = inProgress;
    emit getListListInProgressChanged();
}

void Skywalker::setUnreadNotificationCount(int unread)
{
    const int totalUnread = unread + mNotificationListModel.getInviteCodeUsageNotificationCount();

    if (totalUnread != mUnreadNotificationCount)
    {
        mUnreadNotificationCount = totalUnread;
        emit unreadNotificationCountChanged();
    }
}

// NOTE: indices can be -1 if the UI cannot determine the index
void Skywalker::timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex)
{
    if (mSignOutInProgress)
        return;

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
            auto model = std::make_unique<PostThreadModel>(
                mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
                mMutedWords, mSeenHashtags, this);
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
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "getPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

std::optional<QString> Skywalker::makeOptionalString(const QString& str) const
{
    std::optional<QString> optionalString;
    if (!str.isEmpty())
        optionalString = str;

    return optionalString;
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
    makeLocalModelChange([](LocalPostModelChanges* model){ model->updatePostIndexTimestamps(); });
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalProfileChanges*)>& update)
{
    // Apply change to all active models. When a model gets refreshed (after clear)
    // or deleted, then the local changes will disapper.

    update(&mTimelineModel);

    for (auto& [_, model] : mPostThreadModels.items())
        update(model.get());

    for (auto& [_, model] : mAuthorFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mSearchPostFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mPostFeedModels.items())
        update(model.get());

    if (mBookmarksModel)
        update(mBookmarksModel.get());

    for (auto& [_, model] : mFeedListModels.items())
        update(model.get());

    for (auto& [_, model] : mListListModels.items())
        update(model.get());
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalPostModelChanges*)>& update)
{
    // Apply change to all active models. When a model gets refreshed (after clear)
    // or deleted, then the local changes will disapper.

    update(&mTimelineModel);
    update(&mNotificationListModel);

    for (auto& [_, model] : mPostThreadModels.items())
        update(model.get());

    for (auto& [_, model] : mAuthorFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mSearchPostFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mPostFeedModels.items())
        update(model.get());

    if (mBookmarksModel)
        update(mBookmarksModel.get());
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalAuthorModelChanges*)>& update)
{
    for (auto& [_, model] : mAuthorListModels.items())
        update(model.get());
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalFeedModelChanges*)>& update)
{
    for (auto& [_, model] : mFeedListModels.items())
        update(model.get());
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalListModelChanges*)>& update)
{
    for (auto& [_, model] : mListListModels.items())
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
    mBsky->listNotifications(limit, makeOptionalString(cursor), {},
        [this, cursor](auto list){
            const bool clearFirst = cursor.isEmpty();
            mNotificationListModel.addNotifications(std::move(list), *mBsky, clearFirst);
            setGetNotificationsInProgress(false);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg;
            setGetNotificationsInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
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

void Skywalker::getBookmarksPage(bool clearModel)
{
    Q_ASSERT(mBsky);
    Q_ASSERT(mBookmarksModel);

    if (!mBookmarksModel)
        return;

    if (clearModel)
        mBookmarksModel->clear();

    const int pageIndex = mBookmarksModel->rowCount();
    const auto page = mBookmarks.getPage(pageIndex, BookmarksModel::MAX_PAGE_SIZE);

    if (page.empty())
    {
        qDebug() << "No more bookmarks";
        return;
    }

    mBookmarksModel->addBookmarks(page, *mBsky);
}

void Skywalker::getDetailedProfile(const QString& author)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get detailed profile:" << author;

    mBsky->getProfile(author,
        [this](auto profile){
            auto shared = ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr(profile.release());
            const DetailedProfile detailedProfile(shared);
            AuthorCache::instance().put(detailedProfile);
            emit getDetailedProfileOK(detailedProfile);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getDetailedProfile failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::updateUserProfile(const QString& displayName, const QString& description,
                                  const QString& avatar)
{
    mUserProfile = mUserProfile.nonVolatileCopy();
    mUserProfile.setDisplayName(displayName);
    mUserProfile.setDescription(description);
    mUserProfile.setAvatarUrl(avatar);
    AuthorCache::instance().setUser(mUserProfile);

    makeLocalModelChange(
        [this](LocalProfileChanges* model){ model->updateProfile(mUserProfile); });

    emit avatarUrlChanged();
}

Q_INVOKABLE void Skywalker::getFeedGenerator(const QString& feedUri, bool viewPosts)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get feed generator:" << feedUri;

    mBsky->getFeedGenerator(feedUri,
        [this, viewPosts](auto output){
            auto shared = ATProto::AppBskyFeed::GeneratorView::SharedPtr(output->mView.release());
            emit getFeedGeneratorOK(GeneratorView(shared), viewPosts);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getFeedGenerator failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::clearAuthorFeed(int id)
{
    Q_ASSERT(mBsky);
    qDebug() << "Clear author feed model:" << id;

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

    (*model)->clear();
}

void Skywalker::getAuthorFeed(int id, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author feed model:" << id << "cursor:" << cursor << "max pages:"
             << maxPages << "min entries:" << minEntries;

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
    qDebug() << "Get author feed:" << author.getHandle();

    setGetAuthorFeedInProgress(true);
    mBsky->getAuthorFeed(author.getDid(), limit, makeOptionalString(cursor),
        [this, id, maxPages, minEntries, cursor](auto feed){
            setGetAuthorFeedInProgress(false);
            const auto* model = mAuthorFeedModels.get(id);

            if (!model)
                return; // user has closed the view

            int added = cursor.isEmpty() ?
                    (*model)->setFeed(std::move(feed)) :
                    (*model)->addFeed(std::move(feed));

            // When replies are filtered out, a page can easily become empty
            int entriesToAdd = minEntries - added;

            if (entriesToAdd > 0)
                getAuthorFeedNextPage(id, maxPages - 1, entriesToAdd);
            else
                emit getAuthorFeedOk(id);
        },
        [this, id](const QString& error, const QString& msg){
            setGetAuthorFeedInProgress(false);
            qDebug() << "getAuthorFeed failed:" << error << " - " << msg;
            emit getAuthorFeedFailed(id, error, msg);
        });
}

void Skywalker::getAuthorFeedNextPage(int id, int maxPages, int minEntries)
{
    qDebug() << "Get author feed next page, model:" << id << "max pages:" << maxPages
             << "min entries:" << minEntries;

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
        qDebug() << "End of feed reached.";
        return;
    }

    getAuthorFeed(id, AUTHOR_FEED_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

void Skywalker::getAuthorLikes(int id, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author likes model:" << id << "cursor:" << cursor << "max pages:"
             << maxPages << "min entries:" << minEntries;

    if (mGetAuthorFeedInProgress)
    {
        qDebug() << "Get author likes still in progress";
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
    qDebug() << "Get author likes:" << author.getHandle();

    setGetAuthorFeedInProgress(true);
    mBsky->getActorLikes(author.getDid(), limit, makeOptionalString(cursor),
        [this, id, maxPages, minEntries, cursor](auto feed){
            setGetAuthorFeedInProgress(false);
            const auto* model = mAuthorFeedModels.get(id);

            if (!model)
                return; // user has closed the view

            int added = cursor.isEmpty() ?
                            (*model)->setFeed(std::move(feed)) :
                            (*model)->addFeed(std::move(feed));

            // When replies are filtered out, a page can easily become empty
            int entriesToAdd = minEntries - added;

            if (entriesToAdd > 0)
                getAuthorFeedNextPage(id, maxPages - 1, entriesToAdd);
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorFeedInProgress(false);
            qDebug() << "getAuthorLikes failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorLikesNextPage(int id, int maxPages, int minEntries)
{
    qDebug() << "Get author likes next page, model:" << id << "max pages:" << maxPages
             << "min entries:" << minEntries;

    if (mGetAuthorFeedInProgress)
    {
        qDebug() << "Get author likes still in progress";
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
        qDebug() << "End of feed reached.";
        return;
    }

    getAuthorLikes(id, AUTHOR_LIKES_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

int Skywalker::createAuthorFeedModel(const BasicProfile& author, QEnums::AuthorFeedFilter filter)
{
    auto model = std::make_unique<AuthorFeedModel>(
        author, mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
        mMutedWords, mSeenHashtags, this);
    model->setFilter(filter);
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

int Skywalker::createSearchPostFeedModel()
{
    auto model = std::make_unique<SearchPostFeedModel>(
        mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
        mMutedWords, mSeenHashtags, this);
    const int id = mSearchPostFeedModels.put(std::move(model));
    return id;
}

SearchPostFeedModel* Skywalker::getSearchPostFeedModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mSearchPostFeedModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeSearchPostFeedModel(int id)
{
    qDebug() << "Remove model:" << id;
    mSearchPostFeedModels.remove(id);
}

int Skywalker::createFeedListModel()
{
    auto model = std::make_unique<FeedListModel>(mFavoriteFeeds, this);
    const int id = mFeedListModels.put(std::move(model));
    return id;
}

void Skywalker::getAuthorFeedList(const QString& did, int id, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author feed list:" << id << "did:" << did << "cursor:" << cursor;

    if (mGetFeedInProgress)
    {
        qDebug() << "Get author feed list still in progress";
        return;
    }

    const auto* model = mFeedListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    setGetFeedInProgress(true);
    mBsky->getActorFeeds(did, {}, makeOptionalString(cursor),
        [this, id, cursor](auto output){
            setGetFeedInProgress(false);
            const auto* model = mFeedListModels.get(id);

            if (!model)
                return; // user has closed the view

            if (cursor.isEmpty())
                (*model)->clear();

            (*model)->addFeeds(std::move(output->mFeeds), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetFeedInProgress(false);
            qDebug() << "getAuthorLikes failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorFeedListNextPage(const QString& did, int id)
{
    qDebug() << "Get author feed list next page:" << id << "did:" << did;

    if (mGetFeedInProgress)
    {
        qDebug() << "Get author feed list still in progress";
        return;
    }

    const auto* model = mFeedListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const auto& cursor = (*model)->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    getAuthorFeedList(did, id, cursor);
}

FeedListModel* Skywalker::getFeedListModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mFeedListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeFeedListModel(int id)
{
    qDebug() << "Remove model:" << id;
    mFeedListModels.remove(id);
}

int Skywalker::createPostFeedModel(const GeneratorView& generatorView)
{
    auto model = std::make_unique<PostFeedModel>(generatorView.getDisplayName(),
            mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks, mMutedWords,
            mSeenHashtags, mUserPreferences, mUserSettings, this);
    model->setGeneratorView(generatorView);
    model->enableLanguageFilter(true);
    const int id = mPostFeedModels.put(std::move(model));
    return id;
}

int Skywalker::createPostFeedModel(const ListView& listView)
{
    auto model = std::make_unique<PostFeedModel>(listView.getName(),
                                                 mUserDid, mUserFollows, mMutedReposts,
                                                 mContentFilter, mBookmarks, mMutedWords,
                                                 mSeenHashtags, mUserPreferences, mUserSettings, this);
    model->setListView(listView);
    model->enableLanguageFilter(true);
    const int id = mPostFeedModels.put(std::move(model));
    return id;
}

PostFeedModel* Skywalker::getPostFeedModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mPostFeedModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removePostFeedModel(int id)
{
    qDebug() << "Remove model:" << id;
    mPostFeedModels.remove(id);
}

void Skywalker::getLabelersAuthorList(int modelId)
{
    const std::unordered_set<QString> labelers = mContentFilter.getSubscribedLabelerDids();

    if (labelers.empty())
    {
        qDebug() << "No labelers";
        return;
    }

    std::vector<QString> dids;

    for (const auto& did : labelers)
        dids.push_back(did);

    setGetAuthorListInProgress(true);
    mBsky->getProfiles(dids,
        [this, modelId](auto profileDetailedList){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(profileDetailedList), "");
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getLabelersAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getFollowsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getFollows(atId, limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mFollows), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getFollowsAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getFollowers(atId, limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mFollowers), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getFollowersAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getBlocksAuthorList(int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getBlocks(limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mBlocks), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getBlocksAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getMutesAuthorList(int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getMutes(limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mMutes), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getMutesAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getSuggestionsAuthorList(int limit, const QString& cursor, int modelId)
{
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    setGetAuthorListInProgress(true);
    mBsky->getSuggestions(limit, makeOptionalString(cursor), langs,
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getSuggestionsAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getLikesAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getLikes(atId, limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (!model)
                return;

            ATProto::AppBskyActor::ProfileViewList profileList;

            for (const auto& like : output->mLikes)
                profileList.push_back(std::move(like->mActor));

            (*model)->addAuthors(std::move(profileList), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getLikesAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getRepostsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getRepostedBy(atId, limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mRepostedBy), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getRepostsAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListMembersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getList(atId, limit, makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (!model)
                return;

            (*model)->addAuthors(std::move(output->mItems), output->mCursor.value_or(""));
        },
        [this, atId](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getListMembersAuthorList failed:" << error << " - " << msg;

            // The muted reposts list may not have been created. Consider it as empty.
            if (atId != mUserSettings.getMutedRepostsListUri(mUserDid))
                emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorList(int id, int limit, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author list model:" << id << "cursor:" << cursor;

    if (mGetAuthorListInProgress)
    {
        qDebug() << "Get author list still in progress";
        return;
    }

    const auto* model = mAuthorListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const AuthorListModel::Type type = (*model)->getType();
    const auto& atId = (*model)->getAtId();
    qDebug() << "Get author list:" << atId << "type:" << int(type);

    if (cursor.isEmpty())
        (*model)->clear();

    switch (type)
    {
    case AuthorListModel::Type::AUTHOR_LIST_FOLLOWS:
        getFollowsAuthorList(atId, limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_FOLLOWERS:
        getFollowersAuthorList(atId, limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_BLOCKS:
        getBlocksAuthorList(limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_MUTES:
        getMutesAuthorList(limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_LIKES:
        getLikesAuthorList(atId, limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_REPOSTS:
        getRepostsAuthorList(atId, limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_SEARCH_RESULTS:
        Q_ASSERT(false);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_LIST_MEMBERS:
        getListMembersAuthorList(atId, limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_SUGGESTIONS:
        getSuggestionsAuthorList(limit, cursor, id);
        break;
    case AuthorListModel::Type::AUTHOR_LIST_LABELERS:
        getLabelersAuthorList(id);
        break;
    }
}

void Skywalker::getAuthorListNextPage(int id)
{
    qDebug() << "Get author list next page, model:" << id;

    if (mGetAuthorListInProgress)
    {
        qDebug() << "Get author list still in progress";
        return;
    }

    const auto* model = mAuthorListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    auto* authorListModel = (*model).get();
    const auto& cursor = authorListModel->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "End of list reached.";
        return;
    }

    getAuthorList(id, AUTHOR_LIST_ADD_PAGE_SIZE, cursor);
}

int Skywalker::createAuthorListModel(AuthorListModel::Type type, const QString& atId)
{
    auto model = std::make_unique<AuthorListModel>(type, atId, mMutedReposts, mContentFilter, this);
    const int id = mAuthorListModels.put(std::move(model));
    return id;
}

AuthorListModel* Skywalker::getAuthorListModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mAuthorListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeAuthorListModel(int id)
{
    qDebug() << "Remove model:" << id;
    mAuthorListModels.remove(id);
}

void Skywalker::getListList(int id, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get list list model:" << id << "cursor:" << cursor;

    if (mGetListListInProgress)
    {
        qDebug() << "Get list list still in progress";
        return;
    }

    const auto* model = mListListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const auto type = (*model)->getType();
    const auto& atId = (*model)->getAtId();
    qDebug() << "Get list list:" << atId << "type:" << int(type);

    switch (type) {
    case ListListModel::Type::LIST_TYPE_ALL:
        getListListAll(atId, limit, maxPages, minEntries, cursor, id);
        break;
    case ListListModel::Type::LIST_TYPE_BLOCKS:
        getListListBlocks(limit, maxPages, minEntries, cursor, id);
        break;
    case ListListModel::Type::LIST_TYPE_MUTES:
        getListListMutes(limit, maxPages, minEntries, cursor, id);
        break;
    case ListListModel::Type::LIST_TYPE_SAVED:
        break;
    }
}

void Skywalker::getListListAll(const QString& atId, int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    setGetListListInProgress(true);
    mBsky->getLists(atId, limit, makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            setGetListListInProgress(false);
            qDebug() << "getListListAll succeeded, id:" << modelId;
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this](const QString& error, const QString& msg){
            setGetListListInProgress(false);
            qDebug() << "getListListAll failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListBlocks(int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    setGetListListInProgress(true);
    mBsky->getListBlocks(limit, makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            setGetListListInProgress(false);
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this](const QString& error, const QString& msg){
            setGetListListInProgress(false);
            qDebug() << "getListListBlocks failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListMutes(int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    setGetListListInProgress(true);
    mBsky->getListMutes(limit, makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            setGetListListInProgress(false);
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this](const QString& error, const QString& msg){
            setGetListListInProgress(false);
            qDebug() << "getListListMutes failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListNextPage(int id, int limit, int maxPages, int minEntries)
{
    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    const auto* model = mListListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    const QString& cursor = (*model)->getCursor();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getListList(id, limit, maxPages, minEntries, cursor);
}

int Skywalker::createListListModel(ListListModel::Type type, ListListModel::Purpose purpose, const QString& atId)
{
    auto model = std::make_unique<ListListModel>(type, purpose, atId, mFavoriteFeeds, this, this);
    const int id = mListListModels.put(std::move(model));
    return id;
}

ListListModel* Skywalker::getListListModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mListListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeListListModel(int id)
{
    qDebug() << "Remove model:" << id;
    mListListModels.remove(id);
}

BasicProfile Skywalker::getUser() const
{
    return AuthorCache::instance().getUser();
}

void Skywalker::sharePost(const QString& postUri)
{
    qDebug() << "Share post:" << postUri;
    ATProto::ATUri atUri(postUri);

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("post");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    emit statusMessage(tr("Post link copied to clipboard"));
#endif
}

void Skywalker::shareFeed(const GeneratorView& feed)
{
    qDebug() << "Share feed:" << feed.getDisplayName();
    ATProto::ATUri atUri(feed.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("feed");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    emit statusMessage(tr("Feed link copied to clipboard"));
#endif
}


void Skywalker::shareList(const ListView& list)
{
    qDebug() << "Share list:" << list.getName();
    ATProto::ATUri atUri(list.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("list");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    emit statusMessage(tr("List link copied to clipboard"));
#endif
}

void Skywalker::shareAuthor(const BasicProfile& author)
{
    const QString authorId = author.getDid();
    const QString shareUri = QString("https://bsky.app/profile/%1").arg(authorId);

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("author profile");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    emit statusMessage(tr("Author link copied to clipboard"));
#endif
}

void Skywalker::copyPostTextToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    emit statusMessage(tr("Post text copied to clipboard"));
}

void Skywalker::copyToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    emit statusMessage(tr("Copied to clipboard"));
}

QEnums::ContentVisibility Skywalker::getContentVisibility(const ContentLabelList& contentLabels) const
{
    const auto [visibility, _] = mContentFilter.getVisibilityAndWarning(contentLabels);
    return visibility;
}

QString Skywalker::getContentWarning(const ContentLabelList& contentLabels) const
{
    const auto [_, warning] = mContentFilter.getVisibilityAndWarning(contentLabels);
    return warning;
}

const ContentGroupListModel* Skywalker::getGlobalContentGroupListModel()
{
    mGlobalContentGroupListModel = std::make_unique<ContentGroupListModel>(mContentFilter, this);
    mGlobalContentGroupListModel->setGlobalContentGroups();
    mGlobalContentGroupListModel->setAdultContent(mUserPreferences.getAdultContent());
    return mGlobalContentGroupListModel.get();
}

int Skywalker::createContentGroupListModel(const QString& did, const LabelerPolicies& policies)
{
    auto model = std::make_unique<ContentGroupListModel>(did, mContentFilter, this);
    model->setContentGroups(policies.getContentGroupList());
    model->setAdultContent(mUserPreferences.getAdultContent());
    connect(model.get(), &ContentGroupListModel::error, this, [this](QString error)
            { showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });
    return mContentGroupListModels.put(std::move(model));
}

ContentGroupListModel* Skywalker::getContentGroupListModel(int id) const
{
    qDebug() << "Get model:" << id;
    auto* model = mContentGroupListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeContentGroupListModel(int id)
{
    qDebug() << "Remove model:" << id;
    mContentGroupListModels.remove(id);
}

void Skywalker::saveGlobalContentFilterPreferences()
{
    Q_ASSERT(mGlobalContentGroupListModel);

    if (!mGlobalContentGroupListModel)
    {
        qWarning() << "No filter preferences to save";
        return;
    }

    saveContentFilterPreferences(mGlobalContentGroupListModel.get());
}

void Skywalker::saveContentFilterPreferences(const ContentGroupListModel* model)
{
    Q_ASSERT(model);
    qDebug() << "Save label preferences, labeler DID:" << model->getLabelerDid();

    if (!model->isModified(mUserPreferences))
    {
        qDebug() << "Filter preferences not modified.";
        return;
    }

    auto prefs = mUserPreferences;
    model->saveTo(prefs);
    saveUserPreferences(prefs);
}

ATProto::ProfileMaster& Skywalker::getProfileMaster()
{
    Q_ASSERT(mBsky);

    if (!mProfileMaster)
        mProfileMaster = std::make_unique<ATProto::ProfileMaster>(*mBsky);

    return *mProfileMaster;
}

EditUserPreferences* Skywalker::getEditUserPreferences()
{
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    mEditUserPreferences = std::make_unique<EditUserPreferences>(this);
    mEditUserPreferences->setEmail(session->mEmail.value_or(""));
    mEditUserPreferences->setEmailConfirmed(session->mEmailConfirmed);
    mEditUserPreferences->setEmailAuthFactor(session->mEmailAuthFactor);
    mEditUserPreferences->setDID(mUserDid);
    mEditUserPreferences->setLoggedOutVisibility(mLoggedOutVisibility);
    mEditUserPreferences->setUserPreferences(mUserPreferences);
    mEditUserPreferences->setShowQuotesWithBlockedPost(mUserSettings.getShowQuotesWithBlockedPost(mUserDid));
    mEditUserPreferences->setRewindToLastSeenPost(mUserSettings.getRewindToLastSeenPost(mUserDid));
    mEditUserPreferences->setContentLanguages(mUserSettings.getContentLanguages(mUserDid));
    mEditUserPreferences->setShowUnknownContentLanguage(mUserSettings.getShowUnknownContentLanguage(mUserDid));
    mEditUserPreferences->setShowLanguageTags(mUserSettings.getShowLanguageTags());
    mEditUserPreferences->setDisplayMode(mUserSettings.getDisplayMode());
    mEditUserPreferences->setGifAutoPlay(mUserSettings.getGifAutoPlay());
    mEditUserPreferences->setNotificationsWifiOnly(mUserSettings.getNotificationsWifiOnly());
    mEditUserPreferences->setLocalSettingsModified(false);

    if (session->getPDS())
    {
        const QUrl url(*session->getPDS());
        mEditUserPreferences->setPDS(url.host());
    }
    else
    {
        mEditUserPreferences->setPDS(mBsky->getHost());
    }

    return mEditUserPreferences.get();
}

void Skywalker::saveUserPreferences()
{
    Q_ASSERT(mBsky);
    Q_ASSERT(mEditUserPreferences);

    if (!mEditUserPreferences)
    {
        qWarning() << "No preferences to save";
        return;
    }

    if (mEditUserPreferences->isLocalSettingsModified())
    {
        qDebug() << "Show quotes with blocked posts:" << mEditUserPreferences->getShowQuotesWithBlockedPost();
        mUserSettings.setShowQuotesWithBlockedPost(mUserDid, mEditUserPreferences->getShowQuotesWithBlockedPost());

        qDebug() << "Rewind to last seen post:" << mEditUserPreferences->getRewindToLastSeenPost();
        mUserSettings.setRewindToLastSeenPost(mUserDid, mEditUserPreferences->getRewindToLastSeenPost());

        qDebug() << "Content languages:" << mEditUserPreferences->getContentLanguages();
        mUserSettings.setContentLanguages(mUserDid, mEditUserPreferences->getContentLanguages());

        qDebug() << "Show unknown content language:" << mEditUserPreferences->getShowUnknownContentLanguage();
        mUserSettings.setShowUnknownContentLanguage(mUserDid, mEditUserPreferences->getShowUnknownContentLanguage());

        qDebug() << "Show language tags:" << mEditUserPreferences->getShowLanguageTags();
        mUserSettings.setShowLanguageTags(mEditUserPreferences->getShowLanguageTags());

        qDebug() << "Display mode:" << mEditUserPreferences->getDisplayMode();
        mUserSettings.setDisplayMode(mEditUserPreferences->getDisplayMode());

        qDebug() << "GIF auto play:" << mEditUserPreferences->getGifAutoPlay();
        mUserSettings.setGifAutoPlay(mEditUserPreferences->getGifAutoPlay());

        qDebug() << "Notifications wifi only:" << mEditUserPreferences->getNotificationsWifiOnly();
        mUserSettings.setNotificationsWifiOnly(mEditUserPreferences->getNotificationsWifiOnly());
    }

    const bool loggedOutVisibility = mEditUserPreferences->getLoggedOutVisiblity();
    if (loggedOutVisibility != mLoggedOutVisibility)
    {
        getProfileMaster().setLoggedOutVisibility(mUserDid, loggedOutVisibility,
            [this, loggedOutVisibility]{
                mLoggedOutVisibility = loggedOutVisibility;
            },
            [this](const QString& error, const QString& msg){
                qWarning() << error << " - " << msg;
                emit statusMessage(tr("Failed to change logged-out visibility: ") + msg, QEnums::STATUS_LEVEL_ERROR);
            });
    }

    if (!mEditUserPreferences->isModified())
    {
        qDebug() << "User preferences not modified.";
        return;
    }

    auto prefs = mUserPreferences;
    mEditUserPreferences->saveTo(prefs);
    saveUserPreferences(prefs);
}

const BookmarksModel* Skywalker::createBookmarksModel()
{
    mBookmarksModel = std::make_unique<BookmarksModel>(
        mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
        mMutedWords, mSeenHashtags, this);

    connect(mBookmarksModel.get(), &BookmarksModel::failure, this,
            [this](QString error){ showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });

    return mBookmarksModel.get();
}

void Skywalker::deleteBookmarksModel()
{
    mBookmarksModel = nullptr;
}

DraftPostsModel::Ptr Skywalker::createDraftPostsModel()
{
    return std::make_unique<DraftPostsModel>(
        mUserDid, mUserFollows, mMutedReposts, mContentFilterShowAll, mBookmarks,
        mMutedWordsNoMutes, mSeenHashtags, this);
}

bool Skywalker::sendAppToBackground()
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return false;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("goToBack", "()V");
    return true;
#else
    return false;
#endif
}

void Skywalker::updateUser(const QString& did, const QString& host)
{
    mUserDid = did;
    mUserSettings.addUser(did, host);
    // TODO: password was saved here before.
    mUserSettings.setActiveUserDid(did);
}

void Skywalker::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    mUserSettings.saveSession(session);
}

bool Skywalker::getSavedSession(QString& host, ATProto::ComATProtoServer::Session& session)
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

void Skywalker::saveSyncTimestamp(int postIndex)
{
    if (postIndex < 0 || postIndex >= mTimelineModel.rowCount())
    {
        qWarning() << "Invalid index:" << postIndex << "size:" << mTimelineModel.rowCount();
        return;
    }

    const auto& post = mTimelineModel.getPost(postIndex);
    mUserSettings.saveSyncTimestamp(mUserDid, post.getTimelineTimestamp());
}

QDateTime Skywalker::getSyncTimestamp() const
{
    return mUserSettings.getSyncTimestamp(mUserDid);
}

void Skywalker::shareImage(const QString& contentUri, const QString& text)
{
    if (!FileUtils::checkReadMediaPermission())
    {
        showStatusMessage(tr("No permission to access images."), QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    int fd = FileUtils::openContentUri(contentUri);
    auto [img, error] = PhotoPicker::readImageFd(fd);

    if (img.isNull())
    {
        showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString source = imgProvider->addImage(img);
    emit sharedImageReceived(source, text);
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

void Skywalker::showStatusMessage(const QString& msg, QEnums::StatusLevel level)
{
    emit statusMessage(msg, level);
}

void Skywalker::clearPassword()
{
    if (!mUserDid.isEmpty())
        mUserSettings.clearCredentials(mUserDid);
}

void Skywalker::handleAppStateChange(Qt::ApplicationState state)
{
    qDebug() << "App state:" << state;

    switch (state)
    {
    case Qt::ApplicationSuspended:
        pauseApp();
        break;
    case Qt::ApplicationActive:
        resumeApp();
        break;
    default:
        break;
    };
}

void Skywalker::pauseApp()
{
    qDebug() << "Pause app";

    saveHashtags();
    mUserSettings.setOfflineUnread(mUserDid, mUnreadNotificationCount);
    mUserSettings.setOfflineMessageCheckTimestamp(QDateTime{});
    mUserSettings.resetNextNotificationId();
    mUserSettings.sync();
    OffLineMessageChecker::start(mUserSettings.getNotificationsWifiOnly());

    if (mTimelineUpdateTimer.isActive())
    {
        qDebug() << "Pause timeline auto update";
        stopTimelineAutoUpdate();
        stopRefreshTimers();
        mTimelineUpdatePaused = true;
    }
}

void Skywalker::resumeApp()
{
    qDebug() << "Resume app";

    if (!mTimelineUpdatePaused)
    {
        qDebug() << "Timeline update was not paused.";
        return;
    }

    mTimelineUpdatePaused = false;
    startRefreshTimers();
    startTimelineAutoUpdate();
    refreshSession();
}

void Skywalker::migrateDraftPosts()
{
    if (mUserSettings.isDraftRepoToFileMigrationDone(mUserDid))
    {
        qDebug() << "Draft posts already migrated.";
        emit dataMigrationDone();
        return;
    }

    emit dataMigrationStatus(tr("Migrating drafts"));
    mDraftPostsMigration = std::make_unique<DraftPostsMigration>(this, this);

    connect(mDraftPostsMigration.get(), &DraftPostsMigration::migrationOk, this,
            [this]{
                qDebug() << "Draft posts succesfully migrated";
                mDraftPostsMigration = nullptr;
                mUserSettings.setDraftRepoToFileMigrationDone(mUserDid);
                emit dataMigrationDone();
            });

    connect(mDraftPostsMigration.get(), &DraftPostsMigration::migrationFailed, this,
            [this]{
                qWarning() << "Draft posts migration failed";
                mDraftPostsMigration = nullptr;
                mUserSettings.addDraftRepoToFileMigration(mUserDid);
                showStatusMessage("Could not move (all) draft posts to local storage", QEnums::STATUS_LEVEL_ERROR);
                emit dataMigrationDone();
            });

    mDraftPostsMigration->migrateFromRepoToFile();
}

void Skywalker::signOut()
{
    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
    Q_ASSERT(mSearchPostFeedModels.empty());
    Q_ASSERT(mAuthorListModels.empty());
    Q_ASSERT(mListListModels.empty());
    Q_ASSERT(mPostFeedModels.empty());

    qDebug() << "Logout:" << mUserDid;
    mSignOutInProgress = true;
    saveHashtags();

    stopTimelineAutoUpdate();
    stopRefreshTimers();
    mTimelineUpdatePaused = false;
    mPostThreadModels.clear();
    mAuthorFeedModels.clear();
    mSearchPostFeedModels.clear();
    mAuthorListModels.clear();
    mListListModels.clear();
    mNotificationListModel.clear();
    mUserPreferences = ATProto::UserPreferences();
    mProfileMaster = nullptr;
    mEditUserPreferences = nullptr;
    mGlobalContentGroupListModel = nullptr;
    mTimelineModel.clear();
    mUserDid.clear();
    mUserProfile = {};
    mLoggedOutVisibility = true;
    mUserFollows.clear();
    mMutedReposts.clear();
    setUnreadNotificationCount(0);
    mBookmarksModel = nullptr;
    mBookmarks.clear();
    mMutedWords.clear();
    mUserHashtags.clear();
    mSeenHashtags.clear();
    mFavoriteFeeds.clear();
    mUserSettings.setActiveUserDid({});
    setAutoUpdateTimelineInProgress(false);
    setGetTimelineInProgress(false);
    setGetFeedInProgress(false);
    setGetPostThreadInProgress(false);
    setGetAuthorFeedInProgress(false);
    setGetAuthorListInProgress(false);

    mSignOutInProgress = false;

    emit bskyClientDeleted();
}

}
