// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"
#include "author_cache.h"
#include "chat.h"
#include "display_utils.h"
#include "file_utils.h"
#include "focus_hashtags.h"
#include "font_downloader.h"
#include "jni_callback.h"
#include "list_cache.h"
#include "offline_message_checker.h"
#include "photo_picker.h"
#include "post_thread_cache.h"
#include "shared_image_provider.h"
#include "temp_file_holder.h"
#include "utils.h"
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
static constexpr int TIMELINE_ADD_PAGE_SIZE = 100;
static constexpr int TIMELINE_GAP_FILL_SIZE = 100;
static constexpr int TIMELINE_SYNC_PAGE_SIZE = 100;
static constexpr int TIMELINE_DELETE_SIZE = 100; // must not be smaller than add/sync
static constexpr int FEED_ADD_PAGE_SIZE = 100;
static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 50;
static constexpr int AUTHOR_FEED_ADD_PAGE_SIZE = 100; // Most posts are replies and are filtered
static constexpr int AUTHOR_LIKES_ADD_PAGE_SIZE = 25;
static constexpr int AUTHOR_LIST_ADD_PAGE_SIZE = 50;
static constexpr int USER_HASHTAG_INDEX_SIZE = 100;
static constexpr int SEEN_HASHTAG_INDEX_SIZE = 500;

Skywalker::Skywalker(QObject* parent) :
    IFeedPager(parent),
    mNetwork(new QNetworkAccessManager(this)),
    mTimelineHide(this),
    mUserSettings(this),
    mContentFilter(mUserPreferences, &mUserSettings, this),
    mBookmarks(this),
    mMutedWords(mUserFollows, this),
    mFocusHashtags(new FocusHashtags(this)),
    mGraphUtils(this),
    mNotificationListModel(mContentFilter, mBookmarks, mMutedWords, this),
    mMentionListModel(mContentFilter, mBookmarks, mMutedWords, this),
    mChat(std::make_unique<Chat>(mBsky, mUserDid, this)),
    mUserHashtags(USER_HASHTAG_INDEX_SIZE),
    mSeenHashtags(SEEN_HASHTAG_INDEX_SIZE),
    mFavoriteFeeds(this),
    mAnniversary(mUserDid, mUserSettings, this),
    mTimelineModel(tr("Following"), mUserDid, mUserFollows, mMutedReposts, mTimelineHide,
                   mContentFilter, mBookmarks, mMutedWords, *mFocusHashtags, mSeenHashtags,
                   mUserPreferences, mUserSettings, this)
{
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
    mPlcDirectory = new ATProto::PlcDirectoryClient(mNetwork, ATProto::PlcDirectoryClient::PLC_DIRECTORY_HOST, this);
    mBookmarks.setSkywalker(this);
    mGraphUtils.setSkywalker(this);
    mTimelineHide.setSkywalker(this);
    mTimelineModel.setIsHomeFeed(true);
    connect(&mBookmarks, &Bookmarks::sizeChanged, this, [this]{ mBookmarks.save(); });
    connect(mChat.get(), &Chat::settingsFailed, this, [this](QString error){ showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });
    connect(&mRefreshTimer, &QTimer::timeout, this, [this]{ refreshSession(); });
    connect(&mRefreshNotificationTimer, &QTimer::timeout, this, [this]{ refreshNotificationCount(); });
    connect(&mTimelineUpdateTimer, &QTimer::timeout, this, [this]{ updateTimeline(5, TIMELINE_PREPEND_PAGE_SIZE); });

    connect(&mUserSettings, &UserSettings::backgroundColorChanged, this, [this]{
        const bool isLightMode = mUserSettings.getActiveDisplayMode() == QEnums::DISPLAY_MODE_LIGHT;
        DisplayUtils::setNavigationBarColorAndMode(mUserSettings.getBackgroundColor(), isLightMode);
    });

    AuthorCache::instance().setSkywalker(this);
    AuthorCache::instance().addProfileStore(&mUserFollows);
    ListCache::instance().setSkywalker(this);
    PostThreadCache::instance().setSkywalker(this);
    OffLineMessageChecker::createNotificationChannels();

    auto& jniCallbackListener = JNICallbackListener::getInstance();
    connect(&jniCallbackListener, &JNICallbackListener::sharedTextReceived, this,
            [this](const QString& text){ emit sharedTextReceived(text); });
    connect(&jniCallbackListener, &JNICallbackListener::sharedImageReceived, this,
            [this](const QString& contentUri, const QString& text){ shareImage(contentUri, text); });
    connect(&jniCallbackListener, &JNICallbackListener::sharedVideoReceived, this,
            [this](const QString& contentUri, const QString& text){ shareVideo(contentUri, text); });
    connect(&jniCallbackListener, &JNICallbackListener::sharedDmTextReceived, this,
            [this](const QString& text){ emit sharedDmTextReceived(text); });
    connect(&jniCallbackListener, &JNICallbackListener::showNotifications, this,
            [this]{ emit showNotifications(); });
    connect(&jniCallbackListener, &JNICallbackListener::showDirectMessages, this,
            [this]{ emit showDirectMessages(); });
    connect(&jniCallbackListener, &JNICallbackListener::showLink, this,
            [this](const QString& uri) { emit showLinkReceived(uri); });

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
    qDebug() << "Destructor";
    saveHashtags();

    const auto& emojiFontSource = FontDownloader::getEmojiFontSource();
    if (emojiFontSource.startsWith("file://"))
        TempFileHolder::instance().remove(emojiFontSource.sliced(7));

    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
    Q_ASSERT(mSearchPostFeedModels.empty());
    Q_ASSERT(mAuthorListModels.empty());
    Q_ASSERT(mListListModels.empty());
    Q_ASSERT(mFeedListModels.empty());
    Q_ASSERT(mStarterPackListModels.empty());
    Q_ASSERT(mContentGroupListModels.empty());
}

// NOTE: user can be handle or DID
void Skywalker::login(const QString host, const QString user, QString password, bool rememberPassword, const QString authFactorToken)
{
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->createSession(user, password, Utils::makeOptionalString(authFactorToken),
        [this, host, user, password, rememberPassword]{
            qDebug() << "Login" << user << "succeeded";
            const auto* session = mBsky->getSession();
            updateUser(session->mDid, host);
            saveSession(*session);
            mUserSettings.setRememberPassword(session->mDid, rememberPassword);

            if (rememberPassword)
                mUserSettings.savePassword(session->mDid, password);

            emit loginOk();
            startRefreshTimers();
        },
        [this, host, user, password](const QString& error, const QString& msg){
            qDebug() << "Login" << user << "failed:" << error << " - " << msg;
            mUserSettings.setActiveUserDid({});
            emit loginFailed(error, msg, host, user, password);
        });
}

bool Skywalker::autoLogin()
{
    qDebug() << "Auto login";
    const QString did = mUserSettings.getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return false;
    }

    if (!mUserSettings.getRememberPassword(did))
    {
        qDebug() << "Remember password not enabled";
        return false;
    }

    ATProto::ComATProtoServer::Session session = mUserSettings.getSession(did);

    if (session.mEmailAuthFactor)
    {
        qDebug() << "2FA active";
        return false;
    }

    login("", did, mUserSettings.getPassword(did), true, {});
    return true;
}

bool Skywalker::resumeSession(bool retry)
{
    qDebug() << "Resume session, retry:" << retry;
    const auto session = getSavedSession();

    if (!session)
    {
        qDebug() << "No saved session";
        return false;
    }

    qInfo() << "Session:" << session->mDid << session->mAccessJwt << session->mRefreshJwt;

    auto xrpc = std::make_unique<Xrpc::Client>();
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));

    mBsky->resumeSession(*session,
        [this, retry] {
            qInfo() << "Session resumed";
            saveSession(*mBsky->getSession());
            mUserDid = mBsky->getSession()->mDid;

            if (!retry)
            {
                mBsky->refreshSession(
                    [this]{
                        qDebug() << "Session refreshed";
                        saveSession(*mBsky->getSession());
                        startRefreshTimers();
                        emit resumeSessionOk();
                    },
                    [this](const QString& error, const QString& msg){
                        qDebug() << "Session could not be refreshed:" << error << " - " << msg;
                        mUserSettings.clearTokens(mUserDid);
                        mBsky->clearSession();
                        emit resumeSessionFailed(msg);
                    });
            }
            else
            {
                startRefreshTimers();
                emit resumeSessionOk();
            }
        },
        [this, retry, session](const QString& error, const QString& msg){
            qInfo() << "Session could not be resumed:" << error << " - " << msg;

            if (!retry && error == ATProto::ATProtoErrorMsg::EXPIRED_TOKEN)
            {
                mBsky->setSession(std::make_shared<ATProto::ComATProtoServer::Session>(*session));
                mBsky->refreshSession(
                    [this]{
                        qDebug() << "Session refreshed";
                        saveSession(*mBsky->getSession());
                        resumeSession(true);
                    },
                    [this, did=session->mDid](const QString& error, const QString& msg){
                        qDebug() << "Session could not be refreshed:" << error << " - " << msg;
                        mUserSettings.clearTokens(did);
                        mBsky->clearSession();
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
    auto* session = mBsky->getSession();

    if (!session)
    {
        qWarning() << "No session to delete";
        return;
    }

    const QString did = session->mDid;

    mBsky->deleteSession(
        [this, did]{
            qDebug() << "Session deleted:" << did;
            mUserSettings.clearTokens(did);
            emit sessionDeleted();
        },
        [this, did](const QString& error, const QString& msg){
            qDebug() << "Session could not be deleted:" << did << error << " - " << msg;
            mUserSettings.clearTokens(did);
            mBsky->clearSession();
            emit sessionDeleted();
        });
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
    mGraphUtils.startExpiryCheckTimer();
}

void Skywalker::stopRefreshTimers()
{
    qInfo() << "Refresh timers stopped";
    mRefreshTimer.stop();
    mRefreshNotificationTimer.stop();
    mGraphUtils.stopExpiryCheckTimer();
}

void Skywalker::refreshSession(const std::function<void()>& cbDone)
{
    Q_ASSERT(mBsky);
    qDebug() << "Refresh session";

    const auto* session = mBsky->getSession();
    if (!session)
    {
        qWarning() << "No session to refresh.";

        if (cbDone)
            cbDone();

        stopRefreshTimers();
        emit sessionExpired("Session lost");
        return;
    }

    // TODO: would be nicer to have session refreshment done by the atproto stack
    mBsky->refreshSession(
        [this, cbDone]{
            qDebug() << "Session refreshed";
            saveSession(*mBsky->getSession());

            if (cbDone)
                cbDone();
        },
        [this, cbDone](const QString& error, const QString& msg){
            qDebug() << "Session could not be refreshed:" << error << " - " << msg;

            if (error == ATProto::ATProtoErrorMsg::EXPIRED_TOKEN)
            {
                qWarning() << "Token expired, need to login again";

                if (cbDone)
                    cbDone();

                stopRefreshTimers();
                emit sessionExpired(msg);
            }
            else if (error == ATProto::ATProtoErrorMsg::XRPC_TIMEOUT)
            {
                // Maybe the token got refreshed, but the response never reached us.
                // With the old token we can send one more request, so let's retry.
                // NOTE: this is not fool proof; ideally no other requests should be sent
                // during refresh.
                qDebug() << "Request timed out, retry";
                refreshSession(cbDone);
            }
            else
            {
                qDebug() << "Refresh failed, wait for the next interval to refresh.";

                if (cbDone) {
                    qWarning() << "Refresh failed:" << error << " - " << msg;

                    // There is nothing we can do now. Signal that we are done.
                    // Session will expire later if token is not valid anymore.
                    cbDone();
                }
            }
        });
}

void Skywalker::refreshNotificationCount()
{
    Q_ASSERT(mBsky);
    qDebug() << "Refresh notification count";

    mBsky->getUnreadNotificationCount({}, {},
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
                mUserFollows.add(BasicProfile(profile));

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
                mUserFollows.add(BasicProfile(profile));

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

void Skywalker::signalGetUserProfileOk(ATProto::AppBskyActor::ProfileView::SharedPtr user)
{
    //Q_ASSERT(mUserDid == user->mDid);
    qDebug() << "Got user:" << user->mHandle << "#follows:" << mUserFollows.size();
    AuthorCache::instance().setUser(BasicProfile(user));
    mUserSettings.saveDisplayName(mUserDid, user->mDisplayName.value_or(""));
    const auto avatar = user->mAvatar ? *user->mAvatar : QString();
    mUserSettings.saveAvatar(mUserDid, avatar);
    mLoggedOutVisibility = ATProto::ProfileMaster::getLoggedOutVisibility(*user);
    mUserProfile = Profile(user);

    emit userChanged();
    emit getUserProfileOK();
    setAnniversaryDate();
}

void Skywalker::getUserPreferences()
{
    Q_ASSERT(mBsky);
    qDebug() << "Get user preferences";

    mBsky->getPreferences(
        [this](auto prefs){
            mUserPreferences = prefs;
            emit hideVerificationBadgesChanged();
            updateFavoriteFeeds();
            initLabelers();
            loadLabelSettings();
            mChat->initSettings();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            emit getUserPreferencesFailed(msg);
        });
}

void Skywalker::updateFavoriteFeeds()
{
    qDebug() << "Update favorite feeds";
    const auto searchFeeds = mUserSettings.getPinnedSearchFeeds(mUserDid);
    const auto& savedFeedsPref = mUserPreferences.getSavedFeedsPref();
    mFavoriteFeeds.init(searchFeeds, savedFeedsPref);
}

void Skywalker::saveFavoriteFeeds()
{
    qDebug() << "Save favorite feeds";
    auto prefs = mUserPreferences;
    mFavoriteFeeds.saveTo(prefs, mUserSettings);
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
            const bool oldHideBadges = mUserPreferences.getVerificationPrefs().mHideBadges;
            mUserPreferences = prefs;

            if (mUserPreferences.getVerificationPrefs().mHideBadges != oldHideBadges)
                emit hideVerificationBadgesChanged();

            if (okCb)
                okCb();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "saveUserPreferences failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::loadTimelineHide()
{
    qDebug() << "Load timeline hide lists";
    const QStringList listUris = mUserSettings.getHideLists(mUserDid);
    loadTimelineHide(listUris);
}

void Skywalker::loadTimelineHide(QStringList uris)
{
    Q_ASSERT(mBsky);
    if (uris.empty())
    {
        emit getUserPreferencesOK();
        return;
    }

    const QString uri = uris.back();
    uris.pop_back();

    mTimelineHide.loadList(uri,
        [this, uri, uris]{
            qDebug() << "Loaded:" << uri;
            loadTimelineHide(uris);
        },
        [this, uri, uris](const QString& error, const QString& msg){
            if (ATProto::ATProtoErrorMsg::isListNotFound(error))
            {
                qDebug() << "Hide list not found:" << uri << error << " - " << msg;

                // The list is probbaly delete through another interface. Remove from settings.
                QStringList listUris = mUserSettings.getHideLists(mUserDid);
                listUris.removeOne(uri);
                mUserSettings.setHideLists(mUserDid, listUris);

                loadTimelineHide(uris);
            }
            else
            {
                qWarning() << "Failed:" << error << " - " << msg;
                emit getUserPreferencesFailed(tr("Failed to hide list %1 : %2").arg(uri, msg));
            }
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
        loadTimelineHide();
        return;
    }

    mBsky->getList(uri, 100, Utils::makeOptionalString(cursor),
        [this, maxPages](auto output){
            mMutedReposts.setListCreated(true);

            for (const auto& item : output->mItems)
            {
                const BasicProfile profile(item->mSubject);
                mMutedReposts.add(profile, item->mUri);
            }

            if (output->mCursor)
                loadMutedReposts(maxPages - 1, *output->mCursor);
            else
                loadTimelineHide();
        },
        [this](const QString& error, const QString& msg){
            mMutedReposts.setListCreated(false);

            if (ATProto::ATProtoErrorMsg::isListNotFound(error))
            {
                qDebug() << "No muted reposts list:" << error << " - " << msg;
                loadTimelineHide();
            }
            else
            {
                qWarning() << "loadMutedReposts failed:" << error << " - " << msg;
                emit getUserPreferencesFailed(tr("Failed to load muted reposts: %1").arg(msg));
            }
        });
}

void Skywalker::initLabelers()
{
    Q_ASSERT(mBsky);
    const auto& dids = mContentFilter.getSubscribedLabelerDids();

    if (mBsky->setLabelerDids(dids))
        emit mContentFilter.subscribedLabelersChanged();
}

void Skywalker::loadLabelSettings()
{
    Q_ASSERT(mBsky);
    qDebug() << "Load label settings";
    std::unordered_set<QString> labelerDids = mContentFilter.getSubscribedLabelerDids();
    std::vector<QString> dids(labelerDids.begin(), labelerDids.end());

    if (dids.empty())
    {
        qDebug() << "No labelers";
        loadMutedReposts();
        return;
    }

    mBsky->getServices(dids, true,
        [this, labelerDids](auto output){
            auto remainingDids = labelerDids;
            std::unordered_map<QString, BasicProfile> labelerProfiles;

            for (const auto& v : output->mViews)
            {
                if (v->mViewType != ATProto::AppBskyLabeler::GetServicesOutputView::ViewType::VIEW_DETAILED)
                {
                    qWarning() << "Invalid view type:" << (int)v->mViewType;
                    emit getUserPreferencesFailed(tr("Failed to get labelers: %1").arg("invalid view type"));
                    return;
                }

                const LabelerViewDetailed view(std::get<ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr>(v->mView));
                const auto& policies = view.getPolicies();
                const auto& groupMap = policies.getLabelContentGroupMap();
                const auto creator = view.getCreator();
                const auto& did = creator.getDid();
                qDebug() << "Add label policies for:" << did << view.getCreator().getHandle();
                mContentFilter.addContentGroupMap(did, groupMap);
                remainingDids.erase(did);
                labelerProfiles[did] = view.getCreator();
            }

            if (!remainingDids.empty())
            {
                qWarning() << "Remove subscriptions for non-existing labelers";
                removeLabelerSubscriptions(remainingDids);
            }

            const int notificationCount = mNotificationListModel.addNewLabelsNotifications(labelerProfiles);
            addToUnreadNotificationCount(notificationCount);
            loadMutedReposts();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "initLabelSettings failed:" << error << " - " << msg;
            emit getUserPreferencesFailed(tr("Failed to get labelers: %1").arg(error));
        });
}

void Skywalker::removeLabelerSubscriptions(const std::unordered_set<QString>& dids)
{
    auto userPrefs = mUserPreferences;
    auto labelersPref = userPrefs.getLabelersPref();

    for (const auto& did : dids)
    {
        qWarning() << "Labeler not found:" << did;
        ATProto::AppBskyActor::LabelerPrefItem item;
        item.mDid = did;
        labelersPref.mLabelers.erase(item);
        mUserSettings.removeLabels(mUserDid, did);
    }

    userPrefs.setLabelersPref(labelersPref);
    saveUserPreferences(userPrefs, [this]{ initLabelers(); });
}

void Skywalker::dataMigration()
{
    // Here data migration functions to be executed at startup can be done.
    // dataMigrationStatus can be called to show status during startup.
    emit dataMigrationDone();
}

void Skywalker::syncTimeline(int maxPages)
{
    const auto timestamp = mUserSettings.getSyncTimestamp(mUserDid);

    if (!timestamp.isValid() || !mUserSettings.getRewindToLastSeenPost(mUserDid))
    {
        qDebug() << "Do not rewind timeline";
        getTimeline(TIMELINE_SYNC_PAGE_SIZE);
        finishTimelineSync(-1);
        return;
    }

    // Instant restore does not work well if there are no newer posts saved
    // then the restore points. Timline must still be loaded then, and ListView
    // positioning is when entries get prepended before the timeline that was saved.
    // if (restoreSyncTimelineState())
    // {
    //     finishTimelineSync(mSyncPostIndex);
    //     return;
    // }

    disableDebugLogging(); // sync can cause a lot of logging
    emit timelineSyncStart(maxPages, timestamp);
    const auto cid = mUserSettings.getSyncCid(mUserDid);
    syncTimeline(timestamp, cid, maxPages);
}

void Skywalker::syncListFeed(int modelId, int maxPages)
{
    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    if (model->getFeedType() != QEnums::FEED_LIST)
        return;

    if (!mUserSettings.mustSyncFeed(mUserDid, model->getFeedUri()))
    {
        getListFeed(modelId);
        return;
    }

    const auto timestamp = mUserSettings.getFeedSyncTimestamp(mUserDid, model->getFeedUri());

    if (!timestamp.isValid())
    {
        qDebug() << "Do not rewind timeline";
        getListFeed(modelId);
        return;
    }

    const auto cid = mUserSettings.getFeedSyncCid(mUserDid, model->getFeedUri());
    syncListFeed(modelId, timestamp, cid, maxPages);
}

void Skywalker::syncTimeline(QDateTime tillTimestamp, const QString& cid, int maxPages, const QString& cursor)
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
    mBsky->getTimeline(TIMELINE_SYNC_PAGE_SIZE, Utils::makeOptionalString(cursor),
        [this, tillTimestamp, cid, maxPages, cursor](auto feed){
            const auto newCursor = processSyncPage(std::move(feed), mTimelineModel, tillTimestamp, cid, maxPages, cursor);

            if (!newCursor.isEmpty())
                syncTimeline(tillTimestamp, cid, maxPages - 1, newCursor);
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

QString Skywalker::processSyncPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr feed, PostFeedModel& model, QDateTime tillTimestamp, const QString& cid, int maxPages, const QString& cursor)
{
    if (cursor.isEmpty())
        model.setFeed(std::move(feed));
    else
        model.addFeed(std::move(feed));

    if (model.isHomeFeed())
        setGetTimelineInProgress(false);
    else
        model.setGetFeedInProgress(false);

    const auto lastTimestamp = model.lastTimestamp();

    if (lastTimestamp.isNull())
    {
        restoreDebugLogging();
        qWarning() << "Feed is empty";

        if (model.isHomeFeed())
            finishTimelineSyncFailed();
        else
            finishFeedSyncFailed(model.getModelId());

        return {};
    }

    if (lastTimestamp < tillTimestamp)
    {
        restoreDebugLogging();
        const auto index = model.findTimestamp(tillTimestamp, cid);
        qDebug() << "Feed synced, last timestamp:" << lastTimestamp << "index:"
                 << index << ",feed size:" << model.rowCount()
                 << ",pages left:" << maxPages;

        Q_ASSERT(index >= 0);
        const auto& post = model.getPost(index);
        qDebug() << post.getTimelineTimestamp() << post.getText();

        if (model.isHomeFeed())
            finishTimelineSync(index);
        else
            finishFeedSync(model.getModelId(), index);

        return {};
    }

    if (maxPages == 1)
    {
        restoreDebugLogging();
        qDebug() << "Max pages loaded, failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;

        if (model.isHomeFeed())
            finishTimelineSync(model.rowCount() - 1);
        else
            finishFeedSync(model.getModelId(), model.rowCount() - 1);

        emit statusMessage(tr("Maximum rewind size reached.<br>Cannot rewind till: %1").arg(
                               tillTimestamp.toLocalTime().toString()), QEnums::STATUS_LEVEL_INFO, 10);

        return {};
    }

    const QString& newCursor = model.getLastCursor();

    if (newCursor.isEmpty())
    {
        restoreDebugLogging();
        qDebug() << "Last page reached, no more cursor";

        if (model.isHomeFeed())
            finishTimelineSync(mTimelineModel.rowCount() - 1);
        else {
            finishFeedSync(model.getModelId(), model.rowCount() - 1);
        }

        return {};
    }

    if (newCursor == cursor)
    {
        restoreDebugLogging();
        qWarning() << "New cursor:" << newCursor << "is same as previous:" << cursor;
        qDebug() << "Failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;

        if (model.isHomeFeed())
            finishTimelineSync(mTimelineModel.rowCount() - 1);
        else
            finishFeedSync(model.getModelId(), model.rowCount() - 1);

        return {};
    }

    qInfo() << "Last timestamp:" << lastTimestamp;

    if (model.isHomeFeed())
        emit timelineSyncProgress(maxPages - 1, lastTimestamp);
    else
        emit feedSyncProgress(model.getModelId(), maxPages - 1, lastTimestamp);

    return newCursor;
}

void Skywalker::finishTimelineSync(int index)
{
    mTimelineSynced = true;

    // Inform the GUI about the timeline sync.
    // This will show the timeline to the user.
    const int offsetY = mUserSettings.getSyncOffsetY(mUserDid);
    emit timelineSyncOK(index, offsetY);
    OffLineMessageChecker::checkNotificationPermission();

    // Now we can handle pending intent (content share).
    // If there is any, then this will open the post composition page. This should
    // only been done when the startup sequence in the GUI is finished.
    JNICallbackListener::handlePendingIntent();

    checkAnniversary();
}

void Skywalker::finishTimelineSyncFailed()
{
    emit timelineSyncFailed();
    OffLineMessageChecker::checkNotificationPermission();
    JNICallbackListener::handlePendingIntent();
}

void Skywalker::syncListFeed(int modelId, QDateTime tillTimestamp, const QString& cid, int maxPages, const QString& cursor)
{
    Q_ASSERT(mBsky);
    Q_ASSERT(tillTimestamp.isValid());
    qInfo() << "Sync list feed:" << tillTimestamp << "max pages:" << maxPages;

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qInfo() << "Get feed still in progress";
        return;
    }

    const QString& listUri = model->getListView().getUri();
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    model->setGetFeedInProgress(true);

    if (cursor.isEmpty())
        emit feedSyncStart(modelId, maxPages, tillTimestamp);

    mBsky->getListFeed(listUri, TIMELINE_SYNC_PAGE_SIZE, Utils::makeOptionalString(cursor), langs,
        [this, modelId, tillTimestamp, cid, maxPages, cursor](auto feed){
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            model->setGetFeedInProgress(false);
            const auto newCursor = processSyncPage(std::move(feed), *model, tillTimestamp, cid, maxPages, cursor);

            if (!newCursor.isEmpty())
                syncListFeed(modelId, tillTimestamp, cid, maxPages - 1, newCursor);
        },
        [this, modelId](const QString& error, const QString& msg){
            qWarning() << "Sync feed FAILED:" << error << " - " << msg;

            auto* model = getPostFeedModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }

            finishFeedSyncFailed(modelId);
    });
}

void Skywalker::finishFeedSync(int modelId, int index)
{
    auto* model = getPostFeedModel(modelId);

    if (!model)
        return;

    const int offsetY = mUserSettings.getFeedSyncOffsetY(mUserDid, model->getFeedUri());
    emit feedSyncOk(modelId, index, offsetY);
}

void Skywalker::finishFeedSyncFailed(int modelId)
{
    emit feedSyncFailed(modelId);
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
    mBsky->getTimeline(limit, Utils::makeOptionalString(cursor),
       [this, maxPages, minEntries, cursor](auto feed){
            setGetTimelineInProgress(false);
            int addedPosts = 0;

            if (cursor.isEmpty())
            {
                mTimelineModel.setFeed(std::move(feed));
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
       },
       [this](const QString& error, const QString& msg){
            qInfo() << "getTimeline FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        }
    );
}

void Skywalker::getTimelinePrepend(int autoGapFill, int pageSize, const std::function<void()>& cb)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline prepend, autoGapFill:" << autoGapFill << "pageSize:" << pageSize;

    if (mGetTimelineInProgress)
    {
        qDebug() << "Get timeline still in progress";
        return;
    }

    if (mTimelineModel.rowCount() >= PostFeedModel::MAX_TIMELINE_SIZE)
    {
        qDebug() << "Timeline is full:" << mTimelineModel.rowCount();
        return;
    }

    setAutoUpdateTimelineInProgress(true);
    setGetTimelineInProgress(true);

    mBsky->getTimeline(pageSize, {},
        [this, autoGapFill, cb](auto feed){
            const int gapId = mTimelineModel.prependFeed(std::move(feed));
            setGetTimelineInProgress(false);
            setAutoUpdateTimelineInProgress(false);

            if (gapId > 0)
            {
                if (autoGapFill > 0)
                {
                    getTimelineForGap(gapId, autoGapFill - 1, false, cb);
                    return;
                }
                else
                {
                    qDebug() << "Gap created, no auto gap fill";
                }
            }

            if (cb)
                cb();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "getTimelinePrepend FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            setAutoUpdateTimelineInProgress(false);
            // No need to bother the user with an error message.
            // We will retry on next update/refresh attempt.
        }
        );
}

void Skywalker::getTimelineForGap(int gapId, int autoGapFill, bool userInitiated, const std::function<void()>& cb)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline for gap:" << gapId << "autoGapFill" << autoGapFill;

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
    mBsky->getTimeline(TIMELINE_GAP_FILL_SIZE, cur,
        [this, gapId, autoGapFill, userInitiated, cb](auto feed){
            mTimelineModel.clearLastInsertedRowIndex();
            const int newGapId = mTimelineModel.gapFillFeed(std::move(feed), gapId);
            setGetTimelineInProgress(false);

            if (userInitiated)
            {
                const int gapEndIndex = mTimelineModel.getLastInsertedRowIndex();

                if (gapEndIndex >= 0)
                    emit gapFilled(gapEndIndex);
            }

            if (newGapId > 0)
            {
                if (autoGapFill > 0)
                {
                    getTimelineForGap(newGapId, autoGapFill - 1, userInitiated, cb);
                    return;
                }
                else
                {
                    qDebug() << "Gap created, no auto gap fill";
                }
            }

            if (cb)
                cb();
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
    {
        qInfo() << "Time line size:" << mTimelineModel.rowCount() << "remove head posts:" << TIMELINE_ADD_PAGE_SIZE;
        mTimelineModel.removeHeadPosts(TIMELINE_ADD_PAGE_SIZE);
    }

    getTimeline(TIMELINE_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

void Skywalker::getFeed(int modelId, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get feed model:" << modelId << "cursor:" << cursor;

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    const QString& feedUri = model->getGeneratorView().getUri();
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    model->setGetFeedInProgress(true);

    mBsky->getFeed(feedUri, limit, Utils::makeOptionalString(cursor), langs,
        [this, modelId, maxPages, minEntries, cursor](auto feed){
            int addedPosts = 0;
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            model->setGetFeedInProgress(false);

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
        [this, modelId](const QString& error, const QString& msg){
            qInfo() << "getFeed FAILED:" << error << " - " << msg;
            auto* model = getPostFeedModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }
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

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }


    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    const QString& listUri = model->getListView().getUri();
    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);
    model->setGetFeedInProgress(true);

    mBsky->getListFeed(listUri, limit, Utils::makeOptionalString(cursor), langs,
        [this, modelId, maxPages, minEntries, cursor](auto feed){
            int addedPosts = 0;
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            model->setGetFeedInProgress(false);

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
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getListFeed FAILED:" << error << " - " << msg;
            auto* model = getPostFeedModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }
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

void Skywalker::getQuotesFeed(int modelId, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get quotes feed model:" << modelId << "cursor:" << cursor;

    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    const QString& quoteUri = model->getQuoteUri();
    model->setGetFeedInProgress(true);

    mBsky->getQuotes(quoteUri, {}, limit, Utils::makeOptionalString(cursor),
        [this, modelId, maxPages, minEntries, cursor](auto feed){
            int addedPosts = 0;
            auto* model = getPostFeedModel(modelId);

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            model->setGetFeedInProgress(false);

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
        [this, modelId](const QString& error, const QString& msg){
            qInfo() << "getQuotesFeed FAILED:" << error << " - " << msg;
            auto* model = getPostFeedModel(modelId);

            if (model)
            {
                model->setGetFeedInProgress(false);
                model->setFeedError(msg);
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }
        });
}

void Skywalker::getQuotesFeedNextPage(int modelId, int maxPages, int minEntries)
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

    getQuotesFeed(modelId, FEED_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
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

void Skywalker::setGetPostThreadInProgress(bool inProgress)
{
    mGetPostThreadInProgress = inProgress;
    emit getPostThreadInProgressChanged();
}

void Skywalker::incGetDetailedProfileInProgress()
{
    ++mGetDetailedProfileInProgress;
    emit getDetailedProfileInProgressChanged();
}

void Skywalker::decGetDetailedProfileInProgress()
{
    --mGetDetailedProfileInProgress;
    emit getDetailedProfileInProgressChanged();
}

void Skywalker::setGetNotificationsInProgress(bool inProgress)
{
    mGetNotificationsInProgress = inProgress;
    emit getNotificationsInProgressChanged();
}

void Skywalker::setGetMentionsInProgress(bool inProgress)
{
    mGetMentionsInProgress = inProgress;
    emit getMentionsInProgressChanged();
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

void Skywalker::setGetStarterPackListInProgress(bool inProgress)
{
    mGetStarterPackListInProgress = inProgress;
    emit getStarterPackListInProgressChanged();
}

void Skywalker::setUnreadNotificationCount(int unread)
{
    const int totalUnread = unread + mNotificationListModel.getUnreadCount();

    if (totalUnread != mUnreadNotificationCount)
    {
        mUnreadNotificationCount = totalUnread;
        emit unreadNotificationCountChanged();
    }
}

void Skywalker::addToUnreadNotificationCount(int addUnread)
{
    if (addUnread != 0)
    {
        mUnreadNotificationCount += addUnread;
        emit unreadNotificationCountChanged();
    }
}

// NOTE: indices can be -1 if the UI cannot determine the index
void Skywalker::timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex, int lastVisibleOffsetY)
{
    Q_UNUSED(firstVisibleIndex)

    if (mSignOutInProgress)
        return;

    if (lastVisibleIndex > -1)
        saveSyncTimestamp(lastVisibleIndex, lastVisibleOffsetY);

    const int maxTailSize = mTimelineModel.hasFilters() ? PostFeedModel::MAX_TIMELINE_SIZE * 0.6 : TIMELINE_DELETE_SIZE * 2;

    if (lastVisibleIndex > -1 && mTimelineModel.rowCount() - lastVisibleIndex > maxTailSize)
        mTimelineModel.removeTailPosts(mTimelineModel.rowCount() - lastVisibleIndex - (maxTailSize - TIMELINE_DELETE_SIZE));

    if (lastVisibleIndex > mTimelineModel.rowCount() - TIMELINE_NEXT_PAGE_THRESHOLD && !mGetTimelineInProgress)
        getTimelineNextPage();
}

void Skywalker::feedMovementEnded(int modelId, int lastVisibleIndex, int lastVisibleOffsetY)
{
    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "No model:" << modelId;
        return;
    }

    if (model->getFeedType() != QEnums::FEED_LIST)
        return;

    if (mUserSettings.mustSyncFeed(mUserDid, model->getFeedUri()))
        saveFeedSyncTimestamp(*model, lastVisibleIndex, lastVisibleOffsetY);
}

void Skywalker::getPostThread(const QString& uri, int modelId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get post thread:" << uri << "model:" << modelId;

    if (mGetPostThreadInProgress)
    {
        qDebug() << "Get post thread still in progress";
        return;
    }

    setGetPostThreadInProgress(true);
    mBsky->getPostThread(uri, {}, {},
        [this, uri, modelId](auto thread){
            setGetPostThreadInProgress(false);

            if (modelId < 0)
            {
                auto model = std::make_unique<PostThreadModel>(uri,
                    mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
                    mMutedWords, *mFocusHashtags, mSeenHashtags, this);

                int postEntryIndex = model->setPostThread(thread);

                if (postEntryIndex < 0)
                {
                    qDebug() << "No thread posts";
                    emit statusMessage("Could not create post thread", QEnums::STATUS_LEVEL_ERROR);
                    return;
                }

                const QString uri = model->getPostToAttachMore();
                const int id = addModelToStore<PostThreadModel>(std::move(model), mPostThreadModels);

                if (!uri.isEmpty())
                    addPostThread(uri, id);
                else
                    qDebug() << "No more posts to add";

                emit postThreadOk(id, postEntryIndex);
            }
            else
            {
                auto model = getPostThreadModel(modelId);

                if (model)
                    model->setPostThread(thread);
                else
                    qWarning() << "Model does not exist:" << modelId;
            }
        },
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "getPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::addPostThread(const QString& uri, int modelId, int maxPages)
{
    Q_ASSERT(modelId >= 0);
    qDebug() << "Add post thread:" << uri << "model:" << modelId << "maxPages:" << maxPages;

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";
        return;
    }

    if (mGetPostThreadInProgress)
    {
        qDebug() << "Get post thread still in progress";
        return;
    }

    setGetPostThreadInProgress(true);
    mBsky->getPostThread(uri, {}, 0,
        [this, modelId, maxPages](auto thread){
            setGetPostThreadInProgress(false);
            auto model = getPostThreadModel(modelId);

            if (model)
            {
                if (model->addMorePosts(thread))
                {
                    const QString leafUri = model->getPostToAttachMore();

                    if (!leafUri.isEmpty())
                        addPostThread(leafUri, modelId, maxPages - 1);
                }
                else
                {
                    qDebug() << "No more posts to add";
                }
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }
        },
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "addPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::addOlderPostThread(int modelId)
{
    Q_ASSERT(modelId >= 0);
    qDebug() << "Add older post thread, model:" << modelId;

    if (mGetPostThreadInProgress)
    {
        qDebug() << "Get post thread still in progress";
        return;
    }

    auto model = getPostThreadModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist";
        return;
    }

    const QString rootUri = model->getRootUri();

    if (rootUri.isEmpty())
    {
        qWarning() << "Post thread is empty";
        return;
    }

    setGetPostThreadInProgress(true);
    mBsky->getPostThread(rootUri, 0, {},
        [this, modelId](auto thread){
            setGetPostThreadInProgress(false);
            auto model = getPostThreadModel(modelId);

            if (model)
                model->addOlderPosts(thread);
            else
                qWarning() << "Model does not exist:" << modelId;
        },
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "addOlderPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

PostThreadModel* Skywalker::getPostThreadModel(int id) const
{
    qDebug() << "Get post thread model:" << id;
    auto* model = mPostThreadModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removePostThreadModel(int id)
{
    qDebug() << "Remove model:" << id;
    mPostThreadModels.remove(id);
}

void Skywalker::updatePostIndexedSecondsAgo()
{
    makeLocalModelChange([](LocalPostModelChanges* model){ model->updatePostIndexedSecondsAgo(); });
}

void Skywalker::refreshAllModels()
{
    mTimelineModel.refreshAllData();
    mNotificationListModel.refreshAllData();
    mMentionListModel.refreshAllData();

    for (auto& [_, model] : mPostThreadModels.items())
        model->refreshAllData();

    for (auto& [_, model] : mAuthorFeedModels.items())
        model->refreshAllData();

    for (auto& [_, model] : mSearchPostFeedModels.items())
        model->refreshAllData();

    for (auto& [_, model] : mPostFeedModels.items())
        model->refreshAllData();

    if (mBookmarksModel)
        mBookmarksModel->refreshAllData();

    for (auto& [_, model] : mFeedListModels.items())
        model->refreshAllData();

    for (auto& [_, model] : mListListModels.items())
        model->refreshAllData();

    for (auto& [_, model] : mAuthorListModels.items())
        model->refreshAllData();
}

void Skywalker::makeLocalModelChange(const std::function<void(LocalProfileChanges*)>& update)
{
    // Apply change to all active models. When a model gets refreshed (after clear)
    // or deleted, then the local changes will disapper.

    update(&mTimelineModel);
    mTimelineModel.makeLocalFilteredModelChange(update);
    update(&mNotificationListModel);
    update(&mMentionListModel);

    for (auto& [_, model] : mPostThreadModels.items())
        update(model.get());

    for (auto& [_, model] : mAuthorFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mSearchPostFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mPostFeedModels.items())
    {
        update(model.get());
        model->makeLocalFilteredModelChange(update);
    }

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
    mTimelineModel.makeLocalFilteredModelChange(update);
    update(&mNotificationListModel);
    update(&mMentionListModel);

    for (auto& [_, model] : mPostThreadModels.items())
        update(model.get());

    for (auto& [_, model] : mAuthorFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mSearchPostFeedModels.items())
        update(model.get());

    for (auto& [_, model] : mPostFeedModels.items())
    {
        update(model.get());
        model->makeLocalFilteredModelChange(update);
    }

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

void Skywalker::updateNotificationPreferences(bool priority)
{
    Q_ASSERT(mBsky);
    qDebug() << "Update notification prefereces, priorty:" << priority;

    mBsky->putNotificationPreferences(priority,
        [this]{
            getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, false);
            getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, true);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "updateNotificationPreferences FAILED:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getNotifications(int limit, bool updateSeen, bool mentionsOnly, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get notifications:" << cursor << "mentionsOnly:" << mentionsOnly;
    const bool progress = mentionsOnly ? mGetMentionsInProgress : mGetNotificationsInProgress;

    if (progress)
    {
        qDebug() << "Get notifications still in progress";
        return;
    }

    if (mentionsOnly)
        setGetMentionsInProgress(true);
    else
        setGetNotificationsInProgress(true);

    const auto reasons = mentionsOnly ?
        std::vector<ATProto::AppBskyNotification::NotificationReason>{
            ATProto::AppBskyNotification::NotificationReason::MENTION,
            ATProto::AppBskyNotification::NotificationReason::REPLY,
            ATProto::AppBskyNotification::NotificationReason::QUOTE } :
        std::vector<ATProto::AppBskyNotification::NotificationReason>{};

    mBsky->listNotifications(limit, Utils::makeOptionalString(cursor), {}, {}, reasons,
        [this, mentionsOnly, cursor](auto ouput){
            const bool clearFirst = cursor.isEmpty();
            auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;

            model.addNotifications(std::move(ouput), *mBsky, clearFirst,
                    [this, mentionsOnly, clearFirst]{
                        if (clearFirst)
                        {
                            const auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
                            const int index = model.getIndexOldestUnread();

                            if (index >= 0)
                                emit oldestUnreadNotificationIndex(index, mentionsOnly);
                        }
                    });

            if (mentionsOnly)
                setGetMentionsInProgress(false);
            else
                setGetNotificationsInProgress(false);
        },
        [this, mentionsOnly](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg;

            if (mentionsOnly)
                setGetMentionsInProgress(false);
            else
                setGetNotificationsInProgress(false);

            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        },
        updateSeen);

    if (updateSeen)
    {
        auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
        model.setNotificationsSeen(true);
        setUnreadNotificationCount(0);
    }
}

void Skywalker::getNotificationsNextPage(bool mentionsOnly)
{
    const auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
    const QString& cursor = model.getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, mentionsOnly, cursor);
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
    incGetDetailedProfileInProgress();

    mBsky->getProfile(author,
        [this](auto profile){
            decGetDetailedProfileInProgress();
            const DetailedProfile detailedProfile(profile);
            AuthorCache::instance().put(detailedProfile);
            emit getDetailedProfileOK(detailedProfile);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getDetailedProfile failed:" << error << " - " << msg;
            decGetDetailedProfileInProgress();
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::updateUserProfile(const QString& displayName, const QString& description,
                                  const QString& avatar)
{
    mUserProfile.setDisplayName(displayName);
    mUserProfile.setDescription(description);
    mUserProfile.setAvatarUrl(avatar);
    AuthorCache::instance().setUser(mUserProfile);

    makeLocalModelChange(
        [this](LocalProfileChanges* model){ model->updateProfile(mUserProfile); });

    emit userChanged();
}

void Skywalker::getFeedGenerator(const QString& feedUri, bool viewPosts)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get feed generator:" << feedUri;

    mBsky->getFeedGenerator(feedUri,
        [this, viewPosts](auto output){
            emit getFeedGeneratorOK(GeneratorView(output->mView), viewPosts);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getFeedGenerator failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getStarterPackView(const QString& starterPackUri)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get starter pack view:" << starterPackUri;

    mBsky->getStarterPack(starterPackUri,
        [this](auto starterPackView){
            emit getStarterPackViewOk(StarterPackView(starterPackView));
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getStarterPackView failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::clearAuthorFeed(int id)
{
    Q_ASSERT(mBsky);
    qDebug() << "Clear author feed model:" << id;

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author feed still in progress";
        return;
    }

    (*model)->clear();
}

void Skywalker::getAuthorFeed(int id, int limit, int maxPages, int minEntries, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author feed model:" << id << "cursor:" << cursor << "max pages:"
             << maxPages << "min entries:" << minEntries;

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author feed still in progress";
        return;
    }

    bool includePins = false;
    std::optional<QString> filter;

    switch ((*model)->getFilter())
    {
    case QEnums::AUTHOR_FEED_FILTER_NONE:
        includePins = true;
        break;
    case QEnums::AUTHOR_FEED_FILTER_REPLIES:
        break;
    case QEnums::AUTHOR_FEED_FILTER_POSTS:
        includePins = true;
        filter = ATProto::AppBskyFeed::AuthorFeedFilter::POSTS_NO_REPLIES;
        break;
    case QEnums::AUTHOR_FEED_FILTER_MEDIA:
        filter = ATProto::AppBskyFeed::AuthorFeedFilter::POSTS_WITH_MEDIA;
        break;
    case QEnums::AUTHOR_FEED_FILTER_VIDEO:
        filter = ATProto::AppBskyFeed::AuthorFeedFilter::POSTS_WITH_VIDEO;
        break;
    }

    const auto& author = (*model)->getAuthor();
    qDebug() << "Get author feed:" << author.getHandle();

    (*model)->setGetFeedInProgress(true);
    mBsky->getAuthorFeed(author.getDid(), limit, Utils::makeOptionalString(cursor), filter, includePins,
        [this, id, maxPages, minEntries, cursor](auto feed){
            const auto* model = mAuthorFeedModels.get(id);

            if (!model)
                return; // user has closed the view

            (*model)->setGetFeedInProgress(false);

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
            const auto* model = mAuthorFeedModels.get(id);

            if (model) {
                (*model)->setGetFeedInProgress(false);
                (*model)->setFeedError(msg);
            }

            qDebug() << "getAuthorFeed failed:" << error << " - " << msg;
            emit getAuthorFeedFailed(id, error, msg);
        });
}

void Skywalker::getAuthorFeedNextPage(int id, int maxPages, int minEntries)
{
    qDebug() << "Get author feed next page, model:" << id << "max pages:" << maxPages
             << "min entries:" << minEntries;

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
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

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author likes still in progress";
        return;
    }

    const auto& author = (*model)->getAuthor();
    qDebug() << "Get author likes:" << author.getHandle();

    (*model)->setGetFeedInProgress(true);
    mBsky->getActorLikes(author.getDid(), limit, Utils::makeOptionalString(cursor),
        [this, id, maxPages, minEntries, cursor](auto feed){
            const auto* model = mAuthorFeedModels.get(id);

            if (!model)
                return; // user has closed the view

            (*model)->setGetFeedInProgress(false);

            int added = cursor.isEmpty() ?
                            (*model)->setFeed(std::move(feed)) :
                            (*model)->addFeed(std::move(feed));

            // When replies are filtered out, a page can easily become empty
            int entriesToAdd = minEntries - added;

            if (entriesToAdd > 0)
                getAuthorLikesNextPage(id, maxPages - 1, entriesToAdd);
        },
        [this, id](const QString& error, const QString& msg){
            const auto* model = mAuthorFeedModels.get(id);

            if (model) {
                (*model)->setGetFeedInProgress(false);
                (*model)->setFeedError(msg);
            }

            qDebug() << "getAuthorLikes failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorLikesNextPage(int id, int maxPages, int minEntries)
{
    qDebug() << "Get author likes next page, model:" << id << "max pages:" << maxPages
             << "min entries:" << minEntries;

    const auto* model = mAuthorFeedModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
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

    auto* authorFeedModel = (*model).get();
    const auto& cursor = authorFeedModel->getCursorNextPage();

    if (cursor.isEmpty())
    {
        qDebug() << "End of feed reached.";
        return;
    }

    getAuthorLikes(id, AUTHOR_LIKES_ADD_PAGE_SIZE, maxPages, minEntries, cursor);
}

int Skywalker::createAuthorFeedModel(const DetailedProfile& author, QEnums::AuthorFeedFilter filter)
{
    auto model = std::make_unique<AuthorFeedModel>(
        author, mUserDid, mUserFollows, mMutedReposts, mContentFilter, mBookmarks,
        mMutedWords, *mFocusHashtags, mSeenHashtags, this);
    model->setFilter(filter);
    const int id = addModelToStore<AuthorFeedModel>(std::move(model), mAuthorFeedModels);
    return id;
}

const AuthorFeedModel* Skywalker::getAuthorFeedModel(int id) const
{
    qDebug() << "Get author feed model:" << id;
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
        mMutedWords, *mFocusHashtags, mSeenHashtags, this);
    const int id = addModelToStore<SearchPostFeedModel>(std::move(model), mSearchPostFeedModels);
    return id;
}

SearchPostFeedModel* Skywalker::getSearchPostFeedModel(int id) const
{
    qDebug() << "Get search post feed model:" << id;
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
    qDebug() << "Create feed list model";
    auto model = std::make_unique<FeedListModel>(mFavoriteFeeds, this, this);
    const int id = mFeedListModels.put(std::move(model));
    return id;
}

void Skywalker::getAuthorFeedList(const QString& did, int id, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author feed list:" << id << "did:" << did << "cursor:" << cursor;

    const auto* model = mFeedListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author feed list still in progress";
        return;
    }

    (*model)->setGetFeedInProgress(true);
    mBsky->getActorFeeds(did, {}, Utils::makeOptionalString(cursor),
        [this, id, cursor](auto output){
            const auto* model = mFeedListModels.get(id);

            if (!model)
                return; // user has closed the view

            (*model)->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                (*model)->clear();

            (*model)->addFeeds(std::move(output->mFeeds), output->mCursor.value_or(""));
        },
        [this, id](const QString& error, const QString& msg){
            const auto* model = mFeedListModels.get(id);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->setFeedError(msg);
            }

            qDebug() << "getAuthorLikes failed:" << error << " - " << msg;
        });
}

void Skywalker::getAuthorFeedListNextPage(const QString& did, int id)
{
    qDebug() << "Get author feed list next page:" << id << "did:" << did;

    const auto* model = mFeedListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author feed list still in progress";
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
    qDebug() << "Get feed list model:" << id;
    auto* model = mFeedListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeFeedListModel(int id)
{
    qDebug() << "Remove feed list model:" << id;
    mFeedListModels.remove(id);
}

void Skywalker::getAuthorStarterPackList(const QString& did, int id, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author starter pack list:" << id << "did:" << did << "cursor:" << cursor;

    if (mGetStarterPackListInProgress)
    {
        qDebug() << "Get author starter pack list still in progress";
        return;
    }

    const auto* model = mStarterPackListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    setGetStarterPackListInProgress(true);
    mBsky->getActorStarterPacks(did, {}, Utils::makeOptionalString(cursor),
        [this, id, cursor](auto output){
            setGetStarterPackListInProgress(false);
            const auto* model = mStarterPackListModels.get(id);

            if (!model)
                return; // user has closed the view

            if (cursor.isEmpty())
                (*model)->clear();

            (*model)->addStarterPacks(std::move(output->mStarterPacks), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetStarterPackListInProgress(false);
            qDebug() << "getActorStarterPacks failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorStarterPackListNextPage(const QString& did, int id)
{
    qDebug() << "Get author starter pack list next page:" << id << "did:" << did;

    if (mGetStarterPackListInProgress)
    {
        qDebug() << "Get author feed list still in progress";
        return;
    }

    const auto* model = mStarterPackListModels.get(id);
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

    getAuthorStarterPackList(did, id, cursor);
}

int Skywalker::createStarterPackListModel()
{
    qDebug() << "Create starter pack list model";
    auto model = std::make_unique<StarterPackListModel>(this);
    const int id = mStarterPackListModels.put(std::move(model));
    return id;
}

StarterPackListModel* Skywalker::getStarterPackListModel(int id) const
{
    qDebug() << "Get starter pack model:" << id;
    auto* model = mStarterPackListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeStarterPackListModel(int id)
{
    qDebug() << "Remove starter pack model:" << id;
    mStarterPackListModels.remove(id);
}

template<typename ModelType>
int Skywalker::addModelToStore(ModelType::Ptr model, ItemStore<typename ModelType::Ptr>& store)
{
    auto* modelPtr = model.get();
    const int id = store.put(std::move(model));
    modelPtr->setModelId(id);

    return id;
}

int Skywalker::createPostFeedModel(const GeneratorView& generatorView)
{
    auto model = std::make_unique<PostFeedModel>(generatorView.getDisplayName(),
            mUserDid, mUserFollows, mMutedReposts, ProfileStore::NULL_STORE, mContentFilter, mBookmarks, mMutedWords,
            *mFocusHashtags, mSeenHashtags, mUserPreferences, mUserSettings, this);
    model->setGeneratorView(generatorView);
    model->enableLanguageFilter(true);
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createPostFeedModel(const ListViewBasic& listView)
{
    auto model = std::make_unique<PostFeedModel>(listView.getName(),
                                                 mUserDid, mUserFollows, mMutedReposts, ProfileStore::NULL_STORE,
                                                 mContentFilter, mBookmarks, mMutedWords, *mFocusHashtags,
                                                 mSeenHashtags, mUserPreferences, mUserSettings, this);
    model->setListView(listView);
    model->enableLanguageFilter(true);
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createQuotePostFeedModel(const QString& quoteUri)
{
    auto model = std::make_unique<PostFeedModel>(tr("Quote posts"),
                                                 mUserDid, mUserFollows, mMutedReposts, ProfileStore::NULL_STORE,
                                                 mContentFilter, mBookmarks, mMutedWords, *mFocusHashtags,
                                                 mSeenHashtags, mUserPreferences, mUserSettings, this);
    model->setQuoteUri(quoteUri);
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

PostFeedModel* Skywalker::getPostFeedModel(int id) const
{
    qDebug() << "Get post feed model:" << id;
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
    const std::vector<QString> labelers = mContentFilter.getSubscribedLabelerDidsOrdered();

    if (labelers.empty())
    {
        qDebug() << "No labelers";
        return;
    }

    setGetAuthorListInProgress(true);
    mBsky->getProfiles(labelers,
        [this, modelId](auto profileDetailedList){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->clear();
                (*model)->addAuthors(std::move(profileDetailedList), "");
            }
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
    mBsky->getFollows(atId, limit, Utils::makeOptionalString(cursor),
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
    mBsky->getFollowers(atId, limit, Utils::makeOptionalString(cursor),
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

void Skywalker::getKnownFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getKnownFollowers(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            setGetAuthorListInProgress(false);
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->addAuthors(std::move(output->mFollowers), output->mCursor.value_or(""));
        },
        [this](const QString& error, const QString& msg){
            setGetAuthorListInProgress(false);
            qDebug() << "getKnownFollowersAuthorList failed:" << error << " - " << msg;
            emit statusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getBlocksAuthorList(int limit, const QString& cursor, int modelId)
{
    setGetAuthorListInProgress(true);
    mBsky->getBlocks(limit, Utils::makeOptionalString(cursor),
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
    mBsky->getMutes(limit, Utils::makeOptionalString(cursor),
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
    mBsky->getSuggestions(limit, Utils::makeOptionalString(cursor), langs,
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
    mBsky->getLikes(atId, limit, Utils::makeOptionalString(cursor),
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
    mBsky->getRepostedBy(atId, limit, Utils::makeOptionalString(cursor),
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
    mBsky->getList(atId, limit, Utils::makeOptionalString(cursor),
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
    case AuthorListModel::Type::AUTHOR_LIST_KNOWN_FOLLOWERS:
        getKnownFollowersAuthorList(atId, limit, cursor, id);
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
    auto model = std::make_unique<AuthorListModel>(type, atId, mMutedReposts, mTimelineHide, mContentFilter, this);
    const int id = mAuthorListModels.put(std::move(model));
    return id;
}

AuthorListModel* Skywalker::getAuthorListModel(int id) const
{
    qDebug() << "Get author list model:" << id;
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
    mBsky->getLists(atId, limit, Utils::makeOptionalString(cursor),
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
    mBsky->getListBlocks(limit, Utils::makeOptionalString(cursor),
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
    mBsky->getListMutes(limit, Utils::makeOptionalString(cursor),
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
    qDebug() << "Create list list model";
    auto model = std::make_unique<ListListModel>(type, purpose, atId, mFavoriteFeeds, this, this);
    const int id = mListListModels.put(std::move(model));
    return id;
}

ListListModel* Skywalker::getListListModel(int id) const
{
    qDebug() << "Get list list model:" << id;
    auto* model = mListListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeListListModel(int id)
{
    qDebug() << "Remove list list model:" << id;
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

void Skywalker::shareStarterPack(const StarterPackViewBasic& starterPack)
{
    qDebug() << "Share starter pack:" << starterPack.getName();
    ATProto::ATUri atUri(starterPack.getUri());

    if (!atUri.isValid())
        return;

    const QString shareUri = atUri.toHttpsUri();
    Q_ASSERT(!shareUri.isEmpty());

#ifdef Q_OS_ANDROID
    QJniObject jShareUri = QJniObject::fromString(shareUri);
    QJniObject jSubject = QJniObject::fromString("starter pack");

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/ShareUtils",
                                       "shareLink",
                                       "(Ljava/lang/String;Ljava/lang/String;)V",
                                       jShareUri.object<jstring>(),
                                       jSubject.object<jstring>());
#else
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(shareUri);
    emit statusMessage(tr("Starter pack link copied to clipboard"));
#endif
}

void Skywalker::shareAuthor(const BasicProfile& author)
{
    const QString& authorId = author.getDid();
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

ContentGroup Skywalker::getContentGroup(const QString& did, const QString& labelId) const
{
    const auto* group = mContentFilter.getContentGroup(did, labelId);

    if (group)
        return *group;

    qWarning() << "Uknown label:" << labelId << "did:" << did;
    return ContentGroup(labelId, did);
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
    return mGlobalContentGroupListModel.get();
}

int Skywalker::createContentGroupListModel(const QString& did, const LabelerPolicies& policies)
{
    auto model = std::make_unique<ContentGroupListModel>(did, mContentFilter, this);
    model->setContentGroups(policies.getContentGroupList());
    connect(model.get(), &ContentGroupListModel::error, this, [this](QString error)
            { showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });
    return mContentGroupListModels.put(std::move(model));
}

ContentGroupListModel* Skywalker::getContentGroupListModel(int id) const
{
    qDebug() << "Get content group model:" << id;
    auto* model = mContentGroupListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeContentGroupListModel(int id)
{
    qDebug() << "Remove content group model:" << id;
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
    saveUserPreferences(prefs, [this]{
        initLabelers();
        emit mContentFilter.contentGroupsChanged();
    });
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

    if (!session)
    {
        qWarning() << "Session missing.";
        return nullptr;
    }

    mEditUserPreferences = std::make_unique<EditUserPreferences>(this);
    mEditUserPreferences->setEmail(session->mEmail.value_or(""));
    mEditUserPreferences->setEmailConfirmed(session->mEmailConfirmed);
    mEditUserPreferences->setEmailAuthFactor(session->mEmailAuthFactor);
    mEditUserPreferences->setDID(mUserDid);
    mEditUserPreferences->setLoggedOutVisibility(mLoggedOutVisibility);
    mEditUserPreferences->setUserPreferences(mUserPreferences);

    if (mChat)
        mEditUserPreferences->setAllowIncomingChat(mChat->getAllowIncomingChat());

    if (session->getPDS())
    {
        const QUrl url(*session->getPDS());
        mEditUserPreferences->setPDS(url.host());
    }
    else
    {
        mEditUserPreferences->setPDS(mBsky->getPDS());
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

    const auto allowIncomingChat = mEditUserPreferences->getAllowIncomingChat();
    if (allowIncomingChat != mChat->getAllowIncomingChat())
        mChat->updateSettings(allowIncomingChat);

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
        mMutedWords, *mFocusHashtags, mSeenHashtags, this);

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
        mMutedWordsNoMutes, *mFocusHashtags, mSeenHashtags, this);
}



void Skywalker::updateUser(const QString& did, const QString& host)
{
    mUserDid = did;
    mUserSettings.addUser(did, host);
    mUserSettings.setActiveUserDid(did);
}

void Skywalker::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    mUserSettings.saveSession(session);
}

std::optional<ATProto::ComATProtoServer::Session> Skywalker::getSavedSession() const
{
    const QString did = mUserSettings.getActiveUserDid();

    if (did.isEmpty())
        return {};

    const auto session = mUserSettings.getSession(did);

    if (session.mAccessJwt.isEmpty() || session.mRefreshJwt.isEmpty())
        return {};

    return session;
}

void Skywalker::saveSyncTimestamp(int postIndex, int offsetY)
{
    if (postIndex < 0 || postIndex >= mTimelineModel.rowCount())
    {
        qWarning() << "Invalid index:" << postIndex << "size:" << mTimelineModel.rowCount();
        return;
    }

    const auto& post = mTimelineModel.getPost(postIndex);
    mUserSettings.saveSyncTimestamp(mUserDid, post.getTimelineTimestamp());
    mUserSettings.saveSyncCid(mUserDid, post.getCid());
    mUserSettings.saveSyncOffsetY(mUserDid, offsetY);
}

void Skywalker::saveFeedSyncTimestamp(PostFeedModel& model, int postIndex, int offsetY)
{
    if (postIndex < 0 || postIndex >= model.rowCount())
    {
        qWarning() << "Invalid index:" << postIndex << "size:" << model.rowCount() << model.getFeedName();
        return;
    }

    const auto feedUri = model.getFeedUri();
    qDebug() << "Save feed sync:" << model.getFeedName() << "index:" << postIndex << "offsetY:" << offsetY;

    const auto& post = model.getPost(postIndex);
    mUserSettings.saveFeedSyncTimestamp(mUserDid, feedUri, post.getTimelineTimestamp());
    mUserSettings.saveFeedSyncCid(mUserDid, feedUri, post.getCid());
    mUserSettings.saveFeedSyncOffsetY(mUserDid, feedUri, offsetY);
}

Chat* Skywalker::getChat()
{
    return mChat.get();
}

void Skywalker::shareImage(const QString& contentUri, const QString& text)
{
    if (!FileUtils::checkReadMediaPermission())
    {
        showStatusMessage(tr("No permission to access images."), QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    int fd = FileUtils::openContentUri(contentUri);
    auto [img, gifTempFileName, error] = PhotoPicker::readImageFd(fd);

    if (img.isNull())
    {
        showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
    const QString source = imgProvider->addImage(img);
    emit sharedImageReceived(source, gifTempFileName, text);
}

void Skywalker::shareVideo(const QString& contentUri, const QString& text)
{
    if (!FileUtils::checkReadMediaPermission())
    {
        showStatusMessage(tr("No permission to access video."), QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    // NOTE: the content-URI may point to another video container than mp4, but creating
    // a temp file with mp4-extension seems to work fine for other containers, e.g. webm
    int fd = FileUtils::openContentUri(contentUri);
    auto video = FileUtils::createTempFile(fd, "mp4");

    if (!video)
    {
        showStatusMessage(tr("Could not open video."), QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    const QString tmpFilePath = video->fileName();
    QUrl url = QUrl::fromLocalFile(tmpFilePath);
    TempFileHolder::instance().put(std::move(video));
    emit sharedVideoReceived(url, text);
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

void Skywalker::showStatusMessage(const QString& msg, QEnums::StatusLevel level, int seconds)
{
    emit statusMessage(msg, level, seconds);
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

    if (mBsky && mBsky->getSession())
    {
        // Make sure tokens are saved as the offline message checker needs them
        mUserSettings.saveSession(*mBsky->getSession());
    }

    saveHashtags();
    mUserSettings.setOfflineUnread(mUserDid, mUnreadNotificationCount);
    mUserSettings.setOfflineMessageCheckTimestamp(QDateTime{});
    mUserSettings.setOffLineChatCheckRev(mUserDid, mChat->getLastRev());
    mUserSettings.setCheckOfflineChat(mUserDid, mChat->convosLoaded());
    mUserSettings.resetNextNotificationId();
    mUserSettings.sync();
    OffLineMessageChecker::start(mUserSettings.getNotificationsWifiOnly());

    if (mTimelineUpdateTimer.isActive())
    {
        qDebug() << "Pause timeline auto update";
        stopTimelineAutoUpdate();
        stopRefreshTimers();
        mTimelineUpdatePaused = QDateTime::currentDateTimeUtc();
    }

    mChat->pause();
    emit appPaused();
}

void Skywalker::resumeApp()
{
    qDebug() << "Resume app";

    if (mBsky && mBsky->getSession())
    {
        auto savedSession = mUserSettings.getSession(mUserDid);

        // The offline message checker may have refreshed tokens. Update these tokens
        // so we do not use an old token for refreshing below.
        if (!savedSession.mRefreshJwt.isEmpty())
            mBsky->updateTokens(savedSession.mAccessJwt, savedSession.mRefreshJwt);
        else
            qWarning() << "No tokens";
    }

    if (mTimelineUpdatePaused.isNull())
    {
        qDebug() << "Timeline update was not paused.";
        emit appResumed();
        return;
    }

    const auto pauseInterval = QDateTime::currentDateTimeUtc() - mTimelineUpdatePaused;
    mTimelineUpdatePaused = {};
    const auto lastSyncTimestamp = mUserSettings.getSyncTimestamp(mUserDid);
    const auto lastSyncCid = mUserSettings.getSyncCid(mUserDid);
    const int lastSyncOffsetY = mUserSettings.getSyncOffsetY(mUserDid);
    const auto postCount = mTimelineModel.rowCount();
    qDebug() << "Pause interval:" << pauseInterval << "last sync:" << lastSyncTimestamp << lastSyncCid << "post count:" << postCount;

    refreshSession([this, pauseInterval, lastSyncTimestamp, lastSyncCid, lastSyncOffsetY, postCount]{
        startRefreshTimers();
        startTimelineAutoUpdate();

        if (pauseInterval > 60s)
        {
            updateTimeline(5, 100, [this, lastSyncTimestamp, lastSyncCid, lastSyncOffsetY, postCount]{
                if (postCount != mTimelineModel.rowCount())
                {
                    const int lastSyncIndex = mTimelineModel.findTimestamp(lastSyncTimestamp, lastSyncCid);
                    emit timelineResumed(lastSyncIndex, lastSyncOffsetY);
                }
            });
        }

        mChat->resume();
        emit appResumed();
    });
}

void Skywalker::updateTimeline(int autoGapFill, int pageSize, const std::function<void()>& cb)
{
    qDebug() << "Update timeline, autoGapFill:" << autoGapFill << "pageSize:" << pageSize;
    getTimelinePrepend(autoGapFill, pageSize, cb);
    updatePostIndexedSecondsAgo();
}

void Skywalker::setAnniversaryDate()
{
    Q_ASSERT(!mUserProfile.isNull());
    const auto createdAt = mUserProfile.getCreatedAt();

    if (createdAt.isValid())
    {
        qDebug() << "First appearance set from createdAt:" << createdAt;
        mAnniversary.setFirstAppearance(createdAt);
        checkAnniversary();
        return;
    }

    qDebug() << "Get first appearance from PLC directory";
    mPlcDirectory->getFirstAppearance(mUserProfile.getDid(),
        [this](QDateTime appearance){
            mAnniversary.setFirstAppearance(appearance);
            checkAnniversary();
        },
        {});
}

void Skywalker::checkAnniversary()
{
    if (!mTimelineSynced)
    {
        qDebug() << "Timeline not synced";
        return;
    }

    if (mAnniversary.checkAnniversary())
        emit anniversary();
}

void Skywalker::signOut()
{
    qDebug() << "Sign out:" << mUserDid;
    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
    Q_ASSERT(mSearchPostFeedModels.empty());
    Q_ASSERT(mAuthorListModels.empty());
    Q_ASSERT(mListListModels.empty());
    Q_ASSERT(mFeedListModels.empty());
    Q_ASSERT(mStarterPackListModels.empty());
    Q_ASSERT(mPostFeedModels.empty());

    qDebug() << "Logout:" << mUserDid;
    mSignOutInProgress = true;
    saveHashtags();

    stopTimelineAutoUpdate();
    stopRefreshTimers();
    mTimelineUpdatePaused = {};
    mPostThreadModels.clear();
    mAuthorFeedModels.clear();
    mSearchPostFeedModels.clear();
    mAuthorListModels.clear();
    mListListModels.clear();
    mFeedListModels.clear();
    mStarterPackListModels.clear();
    mPostFeedModels.clear();
    mContentGroupListModels.clear();
    mNotificationListModel.clear();
    mMentionListModel.clear();
    mChat->reset();
    mUserPreferences = ATProto::UserPreferences();
    mProfileMaster = nullptr;
    mEditUserPreferences = nullptr;
    mGlobalContentGroupListModel = nullptr;
    mTimelineModel.reset();
    mUserDid.clear();
    mUserProfile = {};
    mAnniversary.setFirstAppearance({});
    mLoggedOutVisibility = true;
    mUserFollows.clear();
    mMutedReposts.clear();
    mTimelineHide.clear();
    setUnreadNotificationCount(0);
    mBookmarksModel = nullptr;
    mBookmarks.clear();
    mMutedWords.clear();
    mFocusHashtags->clear();
    mUserHashtags.clear();
    mSeenHashtags.clear();
    mFavoriteFeeds.clear();
    mContentFilter.clear();
    mUserSettings.reset();
    mTimelineSynced = false;
    setAutoUpdateTimelineInProgress(false);
    setGetTimelineInProgress(false);
    setGetPostThreadInProgress(false);
    setGetAuthorListInProgress(false);

    mSignOutInProgress = false;

    emit bskyClientDeleted();
}

}
