// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "skywalker.h"
#include "author_cache.h"
#include "chat.h"
#include "definitions.h"
#include "file_utils.h"
#include "filtered_content_post_feed_model.h"
#include "focus_hashtags.h"
#include "font_downloader.h"
#include "jni_callback.h"
#include "list_cache.h"
#include "offline_message_checker.h"
#include "photo_picker.h"
#include "post_thread_cache.h"
#include "search_utils.h"
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
#endif

namespace Skywalker {

using namespace std::chrono_literals;

// There is a trade off: short timeout is fast updating timeline, long timeout
// allows for better reply thread construction as we receive more posts per update.
static constexpr auto TIMELINE_UPDATE_INTERVAL = 91s;

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
    mFollowing(this),
    mFollowsActivityStore(mFollowing, this),
    mTimelineHide(this),
    mUserSettings(this),
    mSessionManager(this, this),
    mContentFilterPolicies(this),
    mContentFilter(mUserDid, mContentFilterPolicies, mUserPreferences, &mUserSettings, this),
    mMutedWords(this),
    mFocusHashtags(new FocusHashtags(this)),
    mGraphUtils(this),
    mNotificationListModel(mContentFilter, mMutedWords, &mFollowsActivityStore, this),
    mMentionListModel(mContentFilter, mMutedWords, &mFollowsActivityStore, this),
    mChat(std::make_unique<Chat>(mBsky, mUserDid, mFollowsActivityStore, this)),
    mUserHashtags(USER_HASHTAG_INDEX_SIZE),
    mSeenHashtags(SEEN_HASHTAG_INDEX_SIZE),
    mFavoriteFeeds(this),
    mAnniversary(mUserDid, mUserSettings, this),
    mTimelineModel(tr("Following"), nullptr, mUserDid, mMutedReposts, mTimelineHide,
                   mContentFilter, mMutedWords, *mFocusHashtags, mSeenHashtags,
                   mUserPreferences, mUserSettings, mFollowsActivityStore, mBsky, this)
{
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
    mPlcDirectory = new ATProto::PlcDirectoryClient(mNetwork, ATProto::PlcDirectoryClient::PLC_DIRECTORY_HOST, this);
    mGraphUtils.setSkywalker(this);
    mTimelineHide.setSkywalker(this);
    mContentFilterPolicies.setSkywalker(this);
    mTimelineModel.setIsHomeFeed(true);

    connect(mChat.get(), &Chat::settingsFailed, this, [this](QString error){ showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });
    connect(&mTimelineUpdateTimer, &QTimer::timeout, this, [this]{ updateTimeline(5, TIMELINE_PREPEND_PAGE_SIZE); });

    connect(&mSessionManager, &SessionManager::activeSessionExpired, this,
        [this](const QString& msg){
            stopRefreshTimers();
            emit sessionExpired(msg);
        });
    connect(&mSessionManager, &SessionManager::totalUnreadNotificationCountChanged, this,
        [this](int unread){ setUnreadNotificationCount(unread); });

    connect(&mUserSettings, &UserSettings::serviceAppViewChanged, this, &Skywalker::updateServiceAppView);
    connect(&mUserSettings, &UserSettings::serviceChatChanged, this, &Skywalker::updateServiceChat);
    connect(&mUserSettings, &UserSettings::serviceVideoHostChanged, this, &Skywalker::updateServiceVideoHost);
    connect(&mUserSettings, &UserSettings::serviceVideoDidChanged, this, &Skywalker::updateServiceVideoDid);

    connect(&mContentFilterPolicies, &ListStore::listRemoved, this,
            [this](const QString& uri){ mUserSettings.removeContentLabelPrefList(mUserDid, uri); });

    AuthorCache::instance().setSkywalker(this);
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

    qDebug() << getUserAgentString();
}

Skywalker::Skywalker(const QString& did, ATProto::Client::SharedPtr bsky, QObject* parent) :
    IFeedPager(parent),
    mNetwork(new QNetworkAccessManager(this)),
    mBsky(bsky),
    mUserDid(did),
    mIsActiveUser(false),
    mFollowsActivityStore(mFollowing, this),
    mTimelineHide(this),
    mUserSettings(this),
    mSessionManager(this, this),
    mContentFilterPolicies(this),
    mContentFilter(mUserDid, mContentFilterPolicies, mUserPreferences, &mUserSettings, this),
    mMutedWords(this),
    mFocusHashtags(new FocusHashtags(this)),
    mGraphUtils(this),
    mNotificationListModel(mContentFilter, mMutedWords, &mFollowsActivityStore, this),
    mMentionListModel(mContentFilter, mMutedWords, &mFollowsActivityStore, this),
    mChat(nullptr),
    mUserHashtags(USER_HASHTAG_INDEX_SIZE),
    mSeenHashtags(SEEN_HASHTAG_INDEX_SIZE),
    mFavoriteFeeds(this),
    mAnniversary(mUserDid, mUserSettings, this),
    mTimelineModel(tr("Following"), nullptr, mUserDid, mMutedReposts, mTimelineHide,
                   mContentFilter, mMutedWords, *mFocusHashtags, mSeenHashtags,
                   mUserPreferences, mUserSettings, mFollowsActivityStore, mBsky, this)
{
    Q_ASSERT(!mUserSettings.getUser(did).isNull());
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
    mPlcDirectory = new ATProto::PlcDirectoryClient(mNetwork, ATProto::PlcDirectoryClient::PLC_DIRECTORY_HOST, this);
    mGraphUtils.setSkywalker(this);
    mTimelineHide.setSkywalker(this);
    mContentFilterPolicies.setSkywalker(this);

    connect(&mUserSettings, &UserSettings::serviceAppViewChanged, this, &Skywalker::updateServiceAppView);
    connect(&mUserSettings, &UserSettings::serviceChatChanged, this, &Skywalker::updateServiceChat);
    connect(&mUserSettings, &UserSettings::serviceVideoHostChanged, this, &Skywalker::updateServiceVideoHost);
    connect(&mUserSettings, &UserSettings::serviceVideoDidChanged, this, &Skywalker::updateServiceVideoDid);

    // The author and post caches are global. When multiple sessions are used
    // this will be mostly fine. The profiles and post content is good. Only
    // labels may be different as different accounts may have different labeler
    // subscriptions.
    // If we want to change this, we need caches per skywalker instance.
}

Skywalker::~Skywalker()
{
    qDebug() << "Destructor";
    emit deleted();
    saveHashtags();

    const auto& emojiFontSource = FontDownloader::getEmojiFontSource();
    if (emojiFontSource.startsWith("file://"))
        TempFileHolder::instance().remove(emojiFontSource.sliced(7));

    mSessionManager.clear();

    Q_ASSERT(mPostThreadModels.empty());
    Q_ASSERT(mAuthorFeedModels.empty());
    Q_ASSERT(mSearchPostFeedModels.empty());
    Q_ASSERT(mAuthorListModels.empty());
    Q_ASSERT(mListListModels.empty());
    Q_ASSERT(mFeedListModels.empty());
    Q_ASSERT(mStarterPackListModels.empty());
    Q_ASSERT(mContentGroupListModels.empty());
    Q_ASSERT(mNotificationListModels.empty());
}

Skywalker::Ptr Skywalker::createSkywalker(const QString& did, ATProto::Client::SharedPtr bsky, QObject* parent)
{
    Skywalker::Ptr skywalker(new Skywalker(did, bsky, parent));

    connect(skywalker.get(), &Skywalker::deleted, this, [this, did]{ emit skywalkerDestroyed(did); });
    connect(skywalker.get(), &Skywalker::statusMessage, this, [this](auto did, auto msg, auto level, auto seconds){ emit statusMessage(did, msg, level, seconds); });
    connect(skywalker.get(), &Skywalker::statusClear, this, [this]{ emit statusClear(); });
    connect(skywalker.get(), &Skywalker::postThreadOk, this, [this](auto did, int id, int entryIndex){ emit postThreadOk(did, id, entryIndex); });
    connect(skywalker.get(), &Skywalker::getDetailedProfileOK, this, [this](auto did, auto profile, auto labelPrefsListUri){ emit getDetailedProfileOK(did, profile, labelPrefsListUri); });
    connect(skywalker.get(), &Skywalker::getFeedGeneratorOK, this, [this](auto did, auto generatorView, bool viewPosts){ emit getFeedGeneratorOK(did, generatorView, viewPosts); });
    connect(skywalker.get(), &Skywalker::getStarterPackViewOk, this, [this](auto did, auto starterPack){ emit getStarterPackViewOk(did, starterPack); });

    emit skywalkerCreated(did, skywalker.get());
    return skywalker;
}

void Skywalker::initNonActiveUser()
{
    Q_ASSERT(!mIsActiveUser);
    qDebug() << "Initialize:" << mUserDid;
    initUserProfile();
    getUserPreferences();
}

QString Skywalker::getUserAgentString()
{
    // NOTE: The "(android)" part is needed for LinkCardReader
    return QString("%1/%2 (android)").arg(APP_NAME, VERSION);
}

// NOTE: user can be handle or DID
void Skywalker::login(const QString host, const QString user, QString password,
                      bool rememberPassword, const QString authFactorToken,
                      bool setAdvancedSettings, const QString serviceAppView,
                      const QString serviceChat, const QString serviceVideoHost,
                      const QString serviceVideoDid)
{
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mBsky = std::make_shared<ATProto::Client>(std::move(xrpc), this);

    mBsky->createSession(user, password, Utils::makeOptionalString(authFactorToken),
        [this, host, user, password, rememberPassword, setAdvancedSettings,
         serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid]{
            qDebug() << "Login" << user << "succeeded";
            const auto* session = mBsky->getSession();
            const QString& did = session->mDid;
            updateUser(did, host);

            if (setAdvancedSettings)
            {
                // These settings will trigger updates on mBsky
                mUserSettings.setServiceAppView(did, serviceAppView);
                mUserSettings.setServiceChat(did, serviceChat);
                mUserSettings.setServiceVideoHost(did, serviceVideoHost);
                mUserSettings.setServiceVideoDid(did, serviceVideoDid);
            }
            else
            {
                mBsky->setServiceAppView(mUserSettings.getServiceAppView(did));
                mBsky->setServiceChat(mUserSettings.getServiceChat(did));
                mBsky->setServiceHostVideo(mUserSettings.getServiceVideoHost(did));
                mBsky->setServiceDidVideo(mUserSettings.getServiceVideoDid(did));
            }

            mUserSettings.saveSession(*session);
            mUserSettings.setRememberPassword(did, rememberPassword); // this calls sync

            if (rememberPassword)
                mUserSettings.savePassword(did, password);

            emit loginOk();

            mSessionManager.insertSession(did, mBsky.get());
            startRefreshTimers();
            mSessionManager.resumeAndRefreshNonActiveUsers();
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

bool Skywalker::resumeAndRefreshSession()
{
    qDebug() << "Resume and refresh session";
    const auto session = getSavedSession();

    if (!session)
    {
        qDebug() << "No saved session";
        return false;
    }

    qDebug() << "Session:" << session->mDid << session->mAccessJwt << session->mRefreshJwt;

    auto xrpc = std::make_unique<Xrpc::Client>();
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mBsky = std::make_shared<ATProto::Client>(std::move(xrpc), this);
    mBsky->setServiceAppView(mUserSettings.getServiceAppView(session->mDid));
    mBsky->setServiceChat(mUserSettings.getServiceChat(session->mDid));
    mBsky->setServiceHostVideo(mUserSettings.getServiceVideoHost(session->mDid));
    mBsky->setServiceDidVideo(mUserSettings.getServiceVideoDid(session->mDid));

    // User DID must be set before inserting sessions in the session manager
    mUserDid = session->mDid;
    mSessionManager.insertSession(session->mDid, mBsky.get());

    mSessionManager.resumeAndRefreshSession(mBsky.get(), *session, 0,
        [this]{
            qDebug() << "Session resumed";
            startRefreshTimers();
            mSessionManager.resumeAndRefreshNonActiveUsers();
            emit resumeSessionOk();
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "Session could not be resumed:" << error << " - " << msg;
            mUserDid.clear();
            emit resumeSessionFailed(msg);
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

    Q_ASSERT(mBsky);
    mSessionManager.startRefreshTimers();
    mGraphUtils.startExpiryCheckTimer();
}

void Skywalker::stopRefreshTimers()
{
    qDebug() << "Refresh timers stopped";
    mSessionManager.stopRefreshTimers();
    mGraphUtils.stopExpiryCheckTimer();
}

void Skywalker::initUserProfile()
{
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qDebug() << "Init user profile, handle:" << session->mHandle << "did:" << session->mDid;

    mBsky->getProfile(session->mDid,
        [this](auto profile){
            qDebug() << "Initialized user profile:" << mUserProfile.getHandle();
            mUserProfile = Profile(profile);
        },
        [this, did=session->mDid](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            auto profile = mUserSettings.getUser(did);
            mUserProfile = Profile(profile.getDid(), profile.getHandle(), profile.getDisplayName(),
                                   profile.getAvatarUrl());
        });
}

void Skywalker::getUserProfileAndFollows()
{
    Q_ASSERT(mBsky);
    const auto* session = mBsky->getSession();
    Q_ASSERT(session);
    qDebug() << "Get user profile, handle:" << session->mHandle << "did:" << session->mDid;

    mBsky->getProfile(session->mDid,
        [this](auto profile){
            signalGetUserProfileOk(profile);
        },
        [this](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            emit getUserProfileFailed(msg);
        });
}

void Skywalker::signalGetUserProfileOk(ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr user)
{
    qDebug() << "Got user:" << user->mHandle;
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
    qDebug() << "Get user preferences:" << mUserDid;

    mBsky->getPreferences(
        [this](auto prefs){
            mUserPreferences = prefs;
            emit hideVerificationBadgesChanged();
            updateFavoriteFeeds();
            initLabelers();
            loadLabelSettings();

            if (mChat)
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
    const auto& savedFeedsPrefV2 = mUserPreferences.getSavedFeedsPrefV2();
    mFavoriteFeeds.init(searchFeeds, savedFeedsPref, savedFeedsPrefV2);
}

void Skywalker::saveFavoriteFeeds()
{
    qDebug() << "Save favorite feeds";
    auto prefs = mUserPreferences;
    mFavoriteFeeds.saveTo(prefs, mUserSettings);
    saveUserPreferences(prefs);
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
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
        loadContentFilterPolicies();
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

                // The list is probbaly deleted through another interface. Remove from settings.
                QStringList listUris = mUserSettings.getHideLists(mUserDid);
                listUris.removeOne(uri);
                mUserSettings.setHideLists(mUserDid, listUris);

                loadTimelineHide(uris);
            }
            else
            {
                qWarning() << "Failed:" << error << " - " << msg;
                emit getUserPreferencesFailed(tr("Failed to load hide list %1 : %2").arg(uri, msg));
            }
        });
}

void Skywalker::loadContentFilterPolicies()
{
    qDebug() << "Load content filter policy lists";
    const QStringList listUris = mUserSettings.getContentLabelPrefListUris(mUserDid);
    loadContentFilterPolicies(listUris);
}

void Skywalker::loadContentFilterPolicies(QStringList uris)
{
    Q_ASSERT(mBsky);
    if (uris.empty())
    {
        qDebug() << "All lists for content filter policies loaded";
        mContentFilter.initListPrefs();
        emit getUserPreferencesOK();
        return;
    }

    const QString uri = uris.back();
    uris.pop_back();

    if (uri == FOLLOWING_LIST_URI)
    {
        qDebug() << "Skip following list";
        loadContentFilterPolicies(uris);
        return;
    }

    mContentFilterPolicies.loadList(uri,
        [this, uri, uris]{
            qDebug() << "Loaded:" << uri;
            loadContentFilterPolicies(uris);
        },
        [this, uri, uris](const QString& error, const QString& msg){
            if (ATProto::ATProtoErrorMsg::isListNotFound(error))
            {
                qDebug() << "Content filter policy list not found:" << uri << error << " - " << msg;

                // The list is probbaly deleted through another interface. Remove from settings.
                mUserSettings.removeContentLabelPrefList(mUserDid, uri);

                loadContentFilterPolicies(uris);
            }
            else
            {
                qWarning() << "Failed:" << error << " - " << msg;
                emit getUserPreferencesFailed(tr("Failed to load hide list %1 : %2").arg(uri, msg));
            }
        });
}

void Skywalker::loadMutedReposts(int maxPages, const QString& cursor)
{
    Q_ASSERT(mBsky);

    if (!mIsActiveUser)
    {
        qDebug() << "Do not load muted repost, not timelineHide lists for other users than the active user";
        loadContentFilterPolicies();
        return;
    }

    qDebug() << "Load muted reposts, maxPages:" << maxPages << "cursor:" << cursor;

    const QString uri = mUserSettings.getMutedRepostsListUri(mUserDid);
    mMutedReposts.setListUri(uri);

    if (maxPages <= 0)
    {
        qWarning() << "Max pages reached";

        emit statusMessage(mUserDid, tr("Too many muted reposts!"), QEnums::STATUS_LEVEL_ERROR);

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

    // The fixed labaler is always included as the Bluesky app has it always enabled and we
    // don't want to erase the label preferences for a fixed labeler.
    std::unordered_set<QString> labelerDids = mContentFilter.getSubscribedLabelerDids(true);
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

            if (mUserSettings.getNewLabelNotifications(mUserDid))
            {
                const int notificationCount = mNotificationListModel.addNewLabelsNotifications(labelerProfiles);
                mSessionManager.setUnreadExtraCount(mUserDid, notificationCount);
            }
            else
            {
                mContentFilter.saveAllNewLabelIdsToSettings();
            }

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
    // Here data migration functions to be executed at startup can be called.
    // dataMigrationStatus can be called to show status during startup.

    // Bookmarks migration will be done while skywalker is running, no need to wait.
    Bookmarks* bookmarks = getBookmarks();
    bookmarks->migrateToBsky();

    emit dataMigrationDone();
}

void Skywalker::syncTimeline(int maxPages)
{
    mTimelineModel.setReverseFeed(mUserSettings.getReverseTimeline(mUserDid));
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

    if (isGetTimelineInProgress())
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
            qWarning() << "syncTimeline FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
        qWarning() << "Feed is empty";

        if (model.isHomeFeed())
            finishTimelineSyncFailed();
        else
            finishFeedSyncFailed(model.getModelId());

        return {};
    }

    if (lastTimestamp < tillTimestamp)
    {
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
        qDebug() << "Max pages loaded, failed to sync till:" << tillTimestamp << "last:" << lastTimestamp;

        if (model.isHomeFeed())
            finishTimelineSync(model.rowCount() - 1);
        else
            finishFeedSync(model.getModelId(), model.rowCount() - 1);

        emit statusMessage(mUserDid, tr("Maximum rewind size reached.<br>Cannot rewind till: %1").arg(
                               tillTimestamp.toLocalTime().toString()), QEnums::STATUS_LEVEL_INFO, 10);

        return {};
    }

    const QString& newCursor = model.getLastCursor();

    if (newCursor.isEmpty())
    {
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
    qDebug() << "Timeline synced";
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
    qWarning() << "Timeline sync failed";
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

    if (isGetTimelineInProgress())
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
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        }
    );
}

void Skywalker::getTimelinePrepend(int autoGapFill, int pageSize, const updateTimelineCb& cb)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline prepend, autoGapFill:" << autoGapFill << "pageSize:" << pageSize;

    if (isGetTimelineInProgress())
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
            qDebug() << "Feed prepended, gapId:" << gapId;

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
            {
                qDebug() << "Callback";
                cb(false);
            }
            else
            {
                qDebug() << "No callback";
            }
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

void Skywalker::getTimelineForGap(int gapId, int autoGapFill, bool userInitiated, const updateTimelineCb& cb)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get timeline for gap:" << gapId << "autoGapFill" << autoGapFill;

    if (isGetTimelineInProgress())
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
                cb(true);
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "getTimelineForGap FAILED:" << error << " - " << msg;
            setGetTimelineInProgress(false);
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
            qDebug() << "getQuotesFeed FAILED:" << error << " - " << msg;
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

void Skywalker::getQuoteChain(int modelId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get quote chain model:" << modelId;

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

    const QString& quoteUri = model->getQuoteUri();
    model->setGetFeedInProgress(true);
    model->clear();

    getQuoteChain(modelId, quoteUri, {});
}

void Skywalker::getQuoteChainNextPage(int modelId)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get quote chain next page model:" << modelId;

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

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get feed still in progress";
        return;
    }

    // The next URI of the next quoted post is set as cursor
    qDebug() << "Cursor:" << cursor;
    model->setGetFeedInProgress(true);
    getQuoteChain(modelId, cursor, {});
}

void Skywalker::getQuoteChain(int modelId, const QString& nextPostUri, std::deque<Post> quoteChain)
{
    qDebug() << "Get quote chain model:" << modelId << "next:" << nextPostUri << "chain size:" << quoteChain.size();

    // The URI will be empty in case of a deleted or blocked post
    if (nextPostUri.isEmpty() || quoteChain.size() >= QUOTE_CHAIN_PAGE_SIZE)
    {
        setQuoteChainInModel(modelId, std::move(quoteChain));
        return;
    }

    mBsky->getPosts({ nextPostUri },
        [this, modelId, quoteChain](auto postViewList){
            if (postViewList.empty())
            {
                qWarning() << "Failed to get posts";

                auto* model = getPostFeedModel(modelId);

                if (model)
                {
                    setQuoteChainInModel(modelId, std::move(quoteChain));
                    model->setFeedError(tr("Failed to get posts"));
                }

                return;
            }

            const Post post(postViewList.front());
            auto chain = std::move(quoteChain);
            chain.push_back(post);

            const auto record = post.getRecordViewFromRecordOrRecordWithMedia();
            QString uri = record ? record->getUri() : "";
            getQuoteChain(modelId, uri, std::move(chain));
        },
        [this, modelId, quoteChain](const QString& error, const QString& msg){
            qDebug() << "getPosts FAILED:" << error << " - " << msg;
            auto* model = getPostFeedModel(modelId);

            if (model)
            {
                setQuoteChainInModel(modelId, std::move(quoteChain));
                model->setFeedError(msg);
            }
            else
            {
                qWarning() << "Model does not exist:" << modelId;
            }
        });
}

void Skywalker::setQuoteChainInModel(int modelId, std::deque<Post> quoteChain)
{
    auto* model = getPostFeedModel(modelId);

    if (!model)
    {
        qWarning() << "Model does not exist:" << modelId;
        return;
    }

    model->setGetFeedInProgress(false);
    model->addQuoteChain(std::move(quoteChain));
}

void Skywalker::setAutoUpdateTimelineInProgress(bool inProgress)
{
    mAutoUpdateTimelineInProgress = inProgress;
    emit autoUpdateTimeLineInProgressChanged();
}

void Skywalker::setGetTimelineInProgress(bool inProgress)
{
    mTimelineModel.setGetFeedInProgress(inProgress);
    emit getTimeLineInProgressChanged();
}

bool Skywalker::isGetTimelineInProgress() const
{
    return mTimelineModel.isGetFeedInProgress();
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

void Skywalker::setUnreadNotificationCount(int unread)
{
    if (unread != mUnreadNotificationCount)
    {
        mUnreadNotificationCount = unread;
        qDebug() << "Unread:" << mUnreadNotificationCount;
        emit unreadNotificationCountChanged();
    }
}

// NOTE: indices can be -1 if the UI cannot determine the index
void Skywalker::timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex, int lastVisibleOffsetY)
{
    Q_UNUSED(firstVisibleIndex)

    if (mSignOutInProgress)
        return;

    if (firstVisibleIndex < 0 || lastVisibleIndex < 0)
        return;

    saveSyncTimestamp(lastVisibleIndex, lastVisibleOffsetY);

    const int maxTailSize = mTimelineModel.hasFilters() ? PostFeedModel::MAX_TIMELINE_SIZE * 0.6 : TIMELINE_DELETE_SIZE * 2;
    const int remainsSize = mTimelineModel.isReverseFeed() ? firstVisibleIndex : mTimelineModel.rowCount() - lastVisibleIndex;

    if (remainsSize > maxTailSize)
        mTimelineModel.removeTailPosts(remainsSize - (maxTailSize - TIMELINE_DELETE_SIZE));

    if (remainsSize < TIMELINE_NEXT_PAGE_THRESHOLD && !isGetTimelineInProgress())
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

void Skywalker::getPostThread(const QString& uri, QEnums::PostThreadType postThreadType)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get post thread:" << uri << "type:" << (int)postThreadType;

    if (mGetPostThreadInProgress)
    {
        qDebug() << "Get post thread still in progress";
        return;
    }

    std::optional<int> parentHeight;

    if (postThreadType == QEnums::POST_THREAD_ENTRY_AUTHOR_POSTS)
        parentHeight = 0;

    setGetPostThreadInProgress(true);
    mBsky->getPostThread(uri, {}, parentHeight,
        [this, uri, postThreadType](auto thread){
            setGetPostThreadInProgress(false);

            auto model = std::make_unique<PostThreadModel>(uri, postThreadType,
                mUserSettings.getReplyOrder(mUserDid),
                mUserSettings.getReplyOrderThreadFirst(mUserDid),
                mUserDid, mMutedReposts, mContentFilter,
                mMutedWords, *mFocusHashtags, mSeenHashtags, this);

            int postEntryIndex = model->setPostThread(thread);

            if (postEntryIndex < 0)
            {
                qDebug() << "No thread posts";
                emit statusMessage(mUserDid, "Could not create post thread", QEnums::STATUS_LEVEL_ERROR);
                return;
            }

            const QString uri = model->getPostToAttachMore();
            const int id = addModelToStore<PostThreadModel>(std::move(model), mPostThreadModels);

            if (!uri.isEmpty())
            {
                addPostThread(uri, id);
            }
            else
            {
                qDebug() << "No more posts to add";

                if (postThreadType == QEnums::POST_THREAD_UNROLLED)
                {
                    auto* m = getPostThreadModel(id);
                    Q_ASSERT(m);

                    if (m)
                        m->unrollThread();
                    else
                        qWarning() << "Model does not exist:" << id;
                }
            }

            emit postThreadOk(mUserDid, id, postEntryIndex);
        },
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "getPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::addPostThread(const QString& uri, int modelId, int maxPages)
{
    Q_ASSERT(modelId >= 0);
    qDebug() << "Add post thread:" << uri << "model:" << modelId << "maxPages:" << maxPages;

    auto model = getPostThreadModel(modelId);

    if (!model)
    {
        qWarning() << "No model:" << modelId;
        return;
    }

    if (maxPages <= 0)
    {
        qDebug() << "Max pages reached";

        if (model->isUnrollThread())
            model->unrollThread();

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

            if (!model)
            {
                qWarning() << "Model does not exist:" << modelId;
                return;
            }

            if (model->addMorePosts(thread))
            {
                const QString leafUri = model->getPostToAttachMore();

                if (!leafUri.isEmpty())
                {
                    addPostThread(leafUri, modelId, maxPages - 1);
                    return;
                }
            }

            qDebug() << "No more posts to add";

            if (model->isUnrollThread())
                model->unrollThread();
        },
        [this](const QString& error, const QString& msg){
            setGetPostThreadInProgress(false);
            qDebug() << "addPostThread FAILED:" << error << " - " << msg;
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

int Skywalker::createPostThreadModel(const QString& uri, QEnums::PostThreadType type)
{
    qDebug() << "Create post thread model:" << uri << "type:" << (int)type;
    auto model = std::make_unique<PostThreadModel>(
        uri, type, mUserSettings.getReplyOrder(mUserDid),
        mUserSettings.getReplyOrderThreadFirst(mUserDid),
        mUserDid, mMutedReposts, mContentFilter,
        mMutedWords, *mFocusHashtags, mSeenHashtags, this);
    const int id = addModelToStore<PostThreadModel>(std::move(model), mPostThreadModels);
    return id;
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

    for (auto& [_, model] : mNotificationListModels.items())
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
    {
        update(model.get());
        model->makeLocalFilteredModelChange(update);
    }

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

    for (auto& [_, model] : mNotificationListModels.items())
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
    {
        update(model.get());
        model->makeLocalFilteredModelChange(update);
    }

    for (auto& [_, model] : mPostFeedModels.items())
    {
        update(model.get());
        model->makeLocalFilteredModelChange(update);
    }

    if (mBookmarksModel)
        update(mBookmarksModel.get());

    for (auto& [_, model] : mNotificationListModels.items())
        update(model.get());
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

void Skywalker::addFeedInteraction(const QString& feedDid, ATProto::AppBskyFeed::Interaction::EventType event,
                        const QString& postUri, const QString& feedContext)
{
    if (feedDid.isEmpty())
        return;

    for (auto& [_, model] : mPostFeedModels.items())
    {
        // In the rare case we have multiple models for a feed, the interaction
        // only needs to be sent once.
        if (model->addFeedInteraction(feedDid, event, postUri, feedContext))
            break;
    }
}

void Skywalker::removeFeedInteraction(const QString& feedDid, ATProto::AppBskyFeed::Interaction::EventType event,
                           const QString& postUri)
{
    if (feedDid.isEmpty())
        return;

    // If there are multiple models for a a feed, make sure that the interaction
    // is removed from all models.
    for (auto& [_, model] : mPostFeedModels.items())
        model->removeFeedInteraction(feedDid, event, postUri);
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
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getNotifications(int limit, bool updateSeen, bool mentionsOnly, bool emitLoadedSignal, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get notifications:" << cursor << "mentionsOnly:" << mentionsOnly;
    auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;

    if (model.isGetFeedInProgress())
    {
        qDebug() << "Get notifications still in progress";
        return;
    }

    model.setGetFeedInProgress(true);

    const auto reasons = mentionsOnly ?
        std::vector<ATProto::AppBskyNotification::NotificationReason>{
            ATProto::AppBskyNotification::NotificationReason::MENTION,
            ATProto::AppBskyNotification::NotificationReason::REPLY,
            ATProto::AppBskyNotification::NotificationReason::QUOTE } :
        std::vector<ATProto::AppBskyNotification::NotificationReason>{};

    mBsky->listNotifications(limit, Utils::makeOptionalString(cursor), {}, {}, reasons,
        [this, mentionsOnly, emitLoadedSignal, cursor](auto ouput){
            const bool clearFirst = cursor.isEmpty();
            auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;

            model.addNotifications(std::move(ouput), mBsky, clearFirst,
                [this, mentionsOnly, emitLoadedSignal]{
                    auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
                    model.setGetFeedInProgress(false);

                    if (emitLoadedSignal)
                    {
                        const int oldestUnreadIndex = model.getIndexOldestUnread();
                        emit unreadNotificationsLoaded(mentionsOnly, oldestUnreadIndex);
                    }
                });
        },
        [this, mentionsOnly](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg;
            auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
            model.setGetFeedInProgress(false);
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        },
        updateSeen);

    if (updateSeen)
    {
        auto& model = mentionsOnly ? mMentionListModel : mNotificationListModel;
        model.setNotificationsSeen(true);
        mSessionManager.setUnreadExtraCount(mUserDid, 0);
        mSessionManager.setUnreadNotificationCount(mUserDid, 0);
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

    getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, mentionsOnly, false, cursor);
}

void Skywalker::getDetailedProfile(const QString& author, const QString& labelPrefsListUri)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get detailed profile:" << author;
    incGetDetailedProfileInProgress();

    mBsky->getProfile(author,
        [this, labelPrefsListUri](auto profile){
            decGetDetailedProfileInProgress();
            const DetailedProfile detailedProfile(profile);
            AuthorCache::instance().put(detailedProfile);
            emit getDetailedProfileOK(mUserDid, detailedProfile, labelPrefsListUri);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getDetailedProfile failed:" << error << " - " << msg;
            decGetDetailedProfileInProgress();
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::updateUserProfile(const QString& displayName, const QString& description,
                                const QString& avatar, const QString& pronouns)
{
    mUserProfile.setDisplayName(displayName);
    mUserProfile.setPronouns(pronouns);
    mUserProfile.setDescription(description);
    mUserProfile.setAvatarUrl(avatar);
    AuthorCache::instance().setUser(mUserProfile);

    makeLocalModelChange(
        [this](LocalProfileChanges* model){ model->updateProfile(mUserProfile); });

    emit userChanged();
}

void Skywalker::updateUserStatus(const ActorStatusView& status)
{
    mUserProfile.setActorStatus(status);
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
            emit getFeedGeneratorOK(mUserDid, GeneratorView(output->mView), viewPosts);
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getFeedGenerator failed:" << error << " - " << msg;
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getStarterPackView(const QString& starterPackUri)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get starter pack view:" << starterPackUri;

    mBsky->getStarterPack(starterPackUri,
        [this](auto starterPackView){
            emit getStarterPackViewOk(mUserDid, StarterPackView(starterPackView));
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getStarterPackView failed:" << error << " - " << msg;
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
    case QEnums::AUTHOR_FEED_FILTER_REPOSTS:
        getAuthorRepostFeed(id, limit, cursor);
        return;
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
            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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

int Skywalker::createNotificationListModel()
{
    auto model = std::make_unique<NotificationListModel>(mContentFilter, mMutedWords, &mFollowsActivityStore, this);
    const int id = addModelToStore<NotificationListModel>(std::move(model), mNotificationListModels);
    return id;
}

NotificationListModel* Skywalker::getNotificationListModel(int id) const
{
    qDebug() << "Get notification list model:" << id;
    auto* model = mNotificationListModels.get(id);
    return model ? model->get() : nullptr;
}

void Skywalker::removeNotificationListModel(int id)
{
    qDebug() << "Remove model:" << id;
    mNotificationListModels.remove(id);
}

int Skywalker::createAuthorFeedModel(const DetailedProfile& author, QEnums::AuthorFeedFilter filter)
{
    auto model = std::make_unique<AuthorFeedModel>(
        author, mUserDid, mMutedReposts, mContentFilter,
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

int Skywalker::createSearchPostFeedModel(const QString& feedName)
{
    auto model = std::make_unique<SearchPostFeedModel>(
        feedName, mUserDid, mMutedReposts, mContentFilter,
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

    const auto* model = mStarterPackListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author starter pack list still in progress";
        return;
    }

    (*model)->setGetFeedInProgress(true);
    mBsky->getActorStarterPacks(did, {}, Utils::makeOptionalString(cursor),
        [this, id, cursor](auto output){
            const auto* model = mStarterPackListModels.get(id);

            if (!model)
                return; // user has closed the view

            (*model)->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                (*model)->clear();

            (*model)->addStarterPacks(std::move(output->mStarterPacks), output->mCursor.value_or(""));
        },
        [this, id](const QString& error, const QString& msg){
            qDebug() << "getActorStarterPacks failed:" << error << " - " << msg;

            const auto* model = mStarterPackListModels.get(id);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorStarterPackListNextPage(const QString& did, int id)
{
    qDebug() << "Get author starter pack list next page:" << id << "did:" << did;

    const auto* model = mStarterPackListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author starter pack list still in progress";
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
    const PostFeedModel::FeedVariant feedVariant{generatorView};
    auto model = std::make_unique<PostFeedModel>(generatorView.getDisplayName(), &feedVariant,
            mUserDid, mMutedReposts, ListStore::NULL_STORE, mContentFilter, mMutedWords,
            *mFocusHashtags, mSeenHashtags, mUserPreferences, mUserSettings, mFollowsActivityStore,
            mBsky, this);
    model->enableLanguageFilter(true);
    model->setReverseFeed(mUserSettings.getFeedReverse(mUserDid, generatorView.getUri()));
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createPostFeedModel(const ListViewBasic& listView)
{
    const PostFeedModel::FeedVariant feedVariant{listView};
    auto model = std::make_unique<PostFeedModel>(listView.getName(), &feedVariant,
                                                 mUserDid, mMutedReposts, ListStore::NULL_STORE,
                                                 mContentFilter, mMutedWords, *mFocusHashtags,
                                                 mSeenHashtags, mUserPreferences, mUserSettings,
                                                 mFollowsActivityStore, mBsky, this);
    model->enableLanguageFilter(true);
    model->setReverseFeed(mUserSettings.getFeedReverse(mUserDid, listView.getUri()));
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createQuotePostFeedModel(const QString& quoteUri)
{
    const PostFeedModel::FeedVariant feedVariant{quoteUri};
    auto model = std::make_unique<PostFeedModel>(tr("Quote posts"), &feedVariant,
                                                 mUserDid, mMutedReposts, ListStore::NULL_STORE,
                                                 mContentFilter, mMutedWords, *mFocusHashtags,
                                                 mSeenHashtags, mUserPreferences, mUserSettings,
                                                 mFollowsActivityStore, mBsky, this);
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createQuoteChainPostFeedModel(const QString& quoteUri)
{
    const PostFeedModel::FeedVariant feedVariant{quoteUri};
    auto model = std::make_unique<PostFeedModel>(tr("Quote chain"), &feedVariant,
                                                 mUserDid, mMutedReposts, ListStore::NULL_STORE,
                                                 mContentFilter, mMutedWords, *mFocusHashtags,
                                                 mSeenHashtags, mUserPreferences, mUserSettings,
                                                 mFollowsActivityStore, mBsky, this);
    const int id = addModelToStore<PostFeedModel>(std::move(model), mPostFeedModels);
    return id;
}

int Skywalker::createFilteredPostFeedModel(QEnums::HideReasonType hideReason, const QString& highlightColor)
{
    const PostFeedModel::FeedVariant feedVariant{hideReason};
    auto model = std::make_unique<FilteredContentPostFeedModel>(
        tr("Filtered posts"), &feedVariant,
        mUserDid, mMutedReposts, ListStore::NULL_STORE,
        mContentFilter, mMutedWords, *mFocusHashtags,
        mSeenHashtags, mUserPreferences, mUserSettings,
        mFollowsActivityStore, mBsky, this);
    model->setHighlightColor(highlightColor);
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

void Skywalker::getAuthorRepostFeed(int id, int limit, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author repost feed model:" << id << "cursor:" << cursor;

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

    const auto& author = (*model)->getAuthor();
    qDebug() << "Get author repost feed:" << author.getHandle();
    const auto profile = author.getProfileBasicView();

    if (!profile)
    {
        qWarning() << "Cannot get profile:" << author.getDid() << author.getHandle();
        return;
    }

    (*model)->setGetFeedInProgress(true);
    postMaster()->getReposts(profile, std::min(limit, ATProto::PostMaster::MAX_GET_REPOSTS), cursor,
        [this, id, cursor](auto feed){
            const auto* model = mAuthorFeedModels.get(id);

            if (!model)
                return;

            (*model)->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                (*model)->setFeed(std::move(feed));
            else
                (*model)->addFeed(std::move(feed));

            emit getAuthorFeedOk(id);
        },
        [this, id](const QString& error, const QString& msg){
            const auto* model = mAuthorFeedModels.get(id);

            if (model) {
                (*model)->setGetFeedInProgress(false);
                (*model)->setFeedError(msg);
            }

            qDebug() << "getAuthorRepostFeed failed:" << error << " - " << msg;
            emit getAuthorFeedFailed(id, error, msg);
        });
}

void Skywalker::getLabelersAuthorList(int modelId)
{
    const std::vector<QString> labelers = mContentFilter.getSubscribedLabelerDidsOrdered();

    if (labelers.empty())
    {
        qDebug() << "No labelers";
        return;
    }

    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getProfiles(labelers,
        [this, modelId](auto profileDetailedList){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->clear();
                std::sort(profileDetailedList.begin(), profileDetailedList.end(),
                    [](const auto& lhs, const auto& rhs){
                        if (lhs->mDid == ContentFilter::BLUESKY_MODERATOR_DID && lhs->mDid != rhs->mDid)
                            return true;

                        if (rhs->mDid == ContentFilter::BLUESKY_MODERATOR_DID && rhs->mDid != lhs->mDid)
                            return false;

                        return SearchUtils::normalizedCompare(
                            lhs->mDisplayName.value_or(lhs->mHandle),
                            rhs->mDisplayName.value_or(rhs->mHandle)) < 0;
                    });
                (*model)->addAuthors(std::move(profileDetailedList), "");
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getLabelersAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getActiveFollowsAuthorList(int modelId, const QString& cursor)
{
    qDebug() << "Get active follows, moded id:" << modelId << "cursor:" << cursor;
    const auto* model = mAuthorListModels.get(modelId);

    if (!model)
    {
        qWarning() << "No active follows model:" << modelId;
        return;
    }

    QString nextCursor = cursor;
    const std::vector<QString> dids = (*model)->getActiveFollowsDids(nextCursor);
    qDebug() << "#did:" << dids.size() << "next cursor:" << nextCursor;

    if (dids.empty())
    {
        qDebug() << "No active follows, cursor:" << cursor;
        return;
    }

    (*model)->setGetFeedInProgress(true);
    mBsky->getProfiles(dids,
        [this, modelId, nextCursor](auto profileDetailedList){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(profileDetailedList), nextCursor);
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getActiveFollowsAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getFollowsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getFollows(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mFollows), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getFollowsAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getFollowers(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mFollowers), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getFollowersAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getKnownFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getKnownFollowers(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mFollowers), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getKnownFollowersAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getBlocksAuthorList(int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getBlocks(limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mBlocks), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getBlocksAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getMutesAuthorList(int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getMutes(limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mMutes), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getMutesAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getActivitySubscriptionsAuthorList(int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->listActivitySubscriptions(limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mSubscriptions), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getActivitySubscriptionsAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getSuggestionsAuthorList(int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    const QStringList langs = mUserSettings.getContentLanguages(mUserDid);

    mBsky->getSuggestions(limit, Utils::makeOptionalString(cursor), langs,
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mActors), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getSuggestionsAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getLikesAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getLikes(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (!model)
                return;

            (*model)->setGetFeedInProgress(false);
            ATProto::AppBskyActor::ProfileView::List profileList;

            for (const auto& like : output->mLikes)
                profileList.push_back(std::move(like->mActor));

            (*model)->addAuthors(std::move(profileList), output->mCursor.value_or(""));
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getLikesAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getRepostsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getRepostedBy(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);
                (*model)->addAuthors(std::move(output->mRepostedBy), output->mCursor.value_or(""));
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getRepostsAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListMembersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId)
{
    const auto* model = mAuthorListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getList(atId, limit, Utils::makeOptionalString(cursor),
        [this, modelId](auto output){
            const auto* model = mAuthorListModels.get(modelId);

            if (!model)
                return;

            (*model)->setGetFeedInProgress(false);
            (*model)->addAuthors(std::move(output->mItems), output->mCursor.value_or(""));
        },
        [this, atId, modelId](const QString& error, const QString& msg){
            qDebug() << "getListMembersAuthorList failed:" << error << " - " << msg;

            const auto* model = mAuthorListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            // The muted reposts list may not have been created. Consider it as empty.
            if (atId != mUserSettings.getMutedRepostsListUri(mUserDid))
                emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getAuthorList(int id, int limit, const QString& cursor)
{
    Q_ASSERT(mBsky);
    qDebug() << "Get author list model:" << id << "cursor:" << cursor;

    const auto* model = mAuthorListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author list still in progress";
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
    case AuthorListModel::Type::AUTHOR_LIST_ACTIVITY_SUBSCRIPTIONS:
        getActivitySubscriptionsAuthorList(limit, cursor, id);
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
    case AuthorListModel::Type::AUTHOR_LIST_ACTIVE_FOLLOWS:
        getActiveFollowsAuthorList(id, cursor);
        break;
    }
}

void Skywalker::getAuthorListNextPage(int id)
{
    qDebug() << "Get author list next page, model:" << id;

    const auto* model = mAuthorListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get author list still in progress";
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
    auto model = std::make_unique<AuthorListModel>(type, atId, mMutedReposts, mTimelineHide,
                                                   mFollowsActivityStore, mContentFilter, this);
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

    const auto* model = mListListModels.get(id);
    Q_ASSERT(model);

    if (!model)
    {
        qWarning() << "Model does not exist:" << id;
        return;
    }

    if ((*model)->isGetFeedInProgress())
    {
        qDebug() << "Get list list still in progress";
        return;
    }

    const auto type = (*model)->getType();
    const auto& atId = (*model)->getAtId();
    qDebug() << "Get list list:" << atId << "type:" << int(type);

    switch (type) {
    case ListListModel::Type::LIST_TYPE_ALL:
        if ((*model)->needsMembershipInfo())
            getListListWithMembershipAll((*model)->getMemberCheckDid(), (*model)->getPurpose(), limit, maxPages, minEntries, cursor, id);
        else
            getListListAll(atId, (*model)->getPurpose(), limit, maxPages, minEntries, cursor, id);

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

void Skywalker::getListListAll(const QString& atId, QEnums::ListPurpose purpose, int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    std::vector<ATProto::AppBskyGraph::ListPurpose> filterPurpose;

    switch (purpose)
    {
    case QEnums::LIST_PURPOSE_MOD:
    case QEnums::LIST_PURPOSE_CURATE:
        filterPurpose.push_back((ATProto::AppBskyGraph::ListPurpose)purpose);
        break;
    default:
        break;
    }

    auto* model = mListListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getLists(atId, filterPurpose, limit, Utils::makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            qDebug() << "getListListAll succeeded, id:" << modelId;
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);

                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getListListAll failed:" << error << " - " << msg;

            const auto* model = mListListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListWithMembershipAll(const QString& atId, QEnums::ListPurpose purpose, int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    qDebug() << "Get lists with membership:" << atId;
    std::vector<ATProto::AppBskyGraph::ListPurpose> filterPurpose;

    switch (purpose)
    {
    case QEnums::LIST_PURPOSE_MOD:
    case QEnums::LIST_PURPOSE_CURATE:
        filterPurpose.push_back((ATProto::AppBskyGraph::ListPurpose)purpose);
        break;
    default:
        break;
    }

    auto* model = mListListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getListsWithMembership(atId, filterPurpose, limit, Utils::makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            qDebug() << "getListListWithMembershipAll succeeded, id:" << modelId;
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);

                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mListsWithMembership), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getListListWithMembershipAll failed:" << error << " - " << msg;

            const auto* model = mListListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListBlocks(int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    auto* model = mListListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getListBlocks(limit, Utils::makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);

                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getListListBlocks failed:" << error << " - " << msg;

            const auto* model = mListListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void Skywalker::getListListMutes(int limit, int maxPages, int minEntries, const QString& cursor, int modelId)
{
    auto* model = mListListModels.get(modelId);

    if (model)
        (*model)->setGetFeedInProgress(true);

    mBsky->getListMutes(limit, Utils::makeOptionalString(cursor),
        [this, modelId, limit, maxPages, minEntries, cursor](auto output){
            const auto* model = mListListModels.get(modelId);

            if (model)
            {
                (*model)->setGetFeedInProgress(false);

                if (cursor.isEmpty())
                    (*model)->clear();

                const int added = (*model)->addLists(std::move(output->mLists), output->mCursor.value_or(""));
                const int toAdd = minEntries - added;

                if (toAdd > 0)
                    getListListNextPage(modelId, limit, maxPages - 1, toAdd);
            }
        },
        [this, modelId](const QString& error, const QString& msg){
            qDebug() << "getListListMutes failed:" << error << " - " << msg;

            const auto* model = mListListModels.get(modelId);

            if (model)
                (*model)->setGetFeedInProgress(false);

            emit statusMessage(mUserDid, msg, QEnums::STATUS_LEVEL_ERROR);
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
    const BasicProfile profile = mUserProfile;
    return profile;
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
    emit statusMessage(mUserDid, tr("Post link copied to clipboard"));
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
    emit statusMessage(mUserDid, tr("Feed link copied to clipboard"));
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
    emit statusMessage(mUserDid, tr("List link copied to clipboard"));
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
    emit statusMessage(mUserDid, tr("Starter pack link copied to clipboard"));
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
    emit statusMessage(mUserDid, tr("Author link copied to clipboard"));
#endif
}

void Skywalker::copyPostTextToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    emit statusMessage(mUserDid, tr("Post text copied to clipboard"));
}

void Skywalker::copyToClipboard(const QString& text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    emit statusMessage(mUserDid, tr("Copied to clipboard"));
}

ContentGroup Skywalker::getContentGroup(const QString& did, const QString& labelId) const
{
    const auto* group = mContentFilter.getContentGroup(did, labelId);

    if (group)
        return *group;

    qWarning() << "Uknown label:" << labelId << "did:" << did;
    return ContentGroup(labelId, did);
}

QEnums::ContentVisibility Skywalker::getContentVisibility(const ContentLabelList& contentLabels, const BasicProfile& author) const
{
    const auto [visibility, _, __] = mContentFilter.getVisibilityAndWarning(author, contentLabels);
    return visibility;
}

QString Skywalker::getContentWarning(const ContentLabelList& contentLabels, const BasicProfile& author) const
{
    const auto [_, warning, __] = mContentFilter.getVisibilityAndWarning(author, contentLabels);
    return warning;
}

QString Skywalker::getContentLabelerDid(const ContentLabelList& contentLabels, const BasicProfile& author) const
{
    const auto [visibility, _, labelIndex] = mContentFilter.getVisibilityAndWarning(author, contentLabels);

    if (visibility == QEnums::CONTENT_VISIBILITY_SHOW)
        return {};

    if (labelIndex < 0 || labelIndex >= contentLabels.size())
        return {};

    return contentLabels[labelIndex].getDid();
}

const ContentGroupListModel* Skywalker::getGlobalContentGroupListModel()
{
    mGlobalContentGroupListModel = std::make_unique<ContentGroupListModel>(mContentFilter, "", this);
    mGlobalContentGroupListModel->setGlobalContentGroups();
    return mGlobalContentGroupListModel.get();
}

int Skywalker::createGlobalContentGroupListModel(const QString& listUri)
{
    auto model = std::make_unique<ContentGroupListModel>(mContentFilter, listUri, this);
    model->setGlobalContentGroups();
    connect(model.get(), &ContentGroupListModel::error, this, [this](QString error)
            { showStatusMessage(error, QEnums::STATUS_LEVEL_ERROR); });
    return mContentGroupListModels.put(std::move(model));
}

int Skywalker::createContentGroupListModel(const QString& labelerDid,
                                           const LabelerPolicies& policies,
                                           const QString& listUri)
{
    auto model = std::make_unique<ContentGroupListModel>(labelerDid, mContentFilter, listUri, this);
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

void Skywalker::saveContentFilterPreferences(ContentGroupListModel* model)
{
    Q_ASSERT(model);
    if (!model)
        return;

    qDebug() << "Save label preferences, labeler DID:" << model->getLabelerDid() << "list:" << model->getListUri();

    if (!model->isModified(mUserPreferences))
    {
        qDebug() << "Filter preferences not modified.";
        return;
    }

    if (model->hasListPrefs())
    {
        model->saveToContentFilter();
        emit mContentFilter.contentGroupsChanged(model->getListUri());
        return;
    }

    auto prefs = mUserPreferences;
    model->saveTo(prefs);
    saveUserPreferences(prefs, [this]{
        initLabelers();
        emit mContentFilter.contentGroupsChanged("");
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
                emit statusMessage(mUserDid, tr("Failed to change logged-out visibility: ") + msg, QEnums::STATUS_LEVEL_ERROR);
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

BookmarksModel* Skywalker::createBookmarksModel()
{
    mBookmarksModel = std::make_unique<BookmarksModel>(
        mUserDid, mMutedReposts, mContentFilter,
        mMutedWords, *mFocusHashtags, mSeenHashtags, this);

    return mBookmarksModel.get();
}

BookmarksModel* Skywalker::getBookmarksModel()
{
    return mBookmarksModel.get();
}

void Skywalker::deleteBookmarksModel()
{
    mBookmarksModel = nullptr;
}

DraftPostsModel::Ptr Skywalker::createDraftPostsModel()
{
    return std::make_unique<DraftPostsModel>(
        mUserDid, mMutedReposts, mContentFilterShowAll,
        mMutedWordsNoMutes, *mFocusHashtags, mSeenHashtags, this);
}



void Skywalker::updateUser(const QString& did, const QString& host)
{
    mUserDid = did;
    mUserSettings.addUser(did, host);
    mUserSettings.setActiveUserDid(did);
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

Bookmarks* Skywalker::getBookmarks()
{
    if (!mBookmarks)
    {
        mBookmarks = std::make_unique<Bookmarks>(this);
        mBookmarks->setSkywalker(this);
    }

    return mBookmarks.get();
}

void Skywalker::shareImage(const QString& contentUri, const QString& text)
{
    if (!FileUtils::checkReadMediaPermission())
    {
        showStatusMessage(tr("No permission to access images."), QEnums::STATUS_LEVEL_ERROR);
        return;
    }

    int fd = FileUtils::openContentUri(contentUri, FileUtils::FileMode::READ_ONLY);
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
    int fd = FileUtils::openContentUri(contentUri, FileUtils::FileMode::READ_ONLY);
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

void Skywalker::showStatusMessage(const QString& msg, QEnums::StatusLevel level, int seconds)
{
    emit statusMessage(mUserDid, msg, level, seconds);
}

void Skywalker::clearStatusMessage()
{
    emit statusClear();
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
        // Also timeline sync timestamps should be saved for sync'ing on startup
        mUserSettings.saveSession(*mBsky->getSession());
        mUserSettings.sync();
    }

    mSessionManager.pause();

    saveHashtags();
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
    mFollowsActivityStore.pause();

    emit appPaused();
}

void Skywalker::resumeApp()
{
    qDebug() << "Resume app";

    if (mUserDid.isEmpty())
    {
        qDebug() << "No user active";
        emit appResumed();
        return;
    }

    mSessionManager.resume();

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
    const int lastSyncIndex = mTimelineModel.findTimestamp(lastSyncTimestamp, lastSyncCid);
    qDebug() << "Pause interval:" << pauseInterval << "last sync:" << lastSyncTimestamp << lastSyncCid << "index:" << lastSyncIndex;

    mBsky->autoRefreshSession([this, pauseInterval, lastSyncTimestamp, lastSyncCid, lastSyncOffsetY, lastSyncIndex]{
        startRefreshTimers();
        startTimelineAutoUpdate();

        if (pauseInterval > 60s)
        {
            updateTimeline(5, 100, [this, lastSyncTimestamp, lastSyncCid, lastSyncOffsetY, lastSyncIndex](bool gapFilled){
                // If a gap was filled, then mulitple pages have been prepended to the timeline.
                // That may cause the position to get lost due to scrolling on prepends when the
                // position was near the top when the app was paused.
                // Signal that the update is finished so th GUI can reposition.
                if (gapFilled && lastSyncIndex <= 5)
                {
                    const int newSyncIndex = mTimelineModel.findTimestamp(lastSyncTimestamp, lastSyncCid);
                    emit timelineResumed(newSyncIndex, lastSyncOffsetY);
                }
            });
        }

        mChat->resume();
        mFollowsActivityStore.resume();

        emit appResumed();
    });
}

void Skywalker::updateTimeline(int autoGapFill, int pageSize, const updateTimelineCb& cb)
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

void Skywalker::updateServiceAppView(const QString& did)
{
    if (mBsky && mBsky->getSessionDid() == did)
        mBsky->setServiceAppView(mUserSettings.getServiceAppView(did));
}

void Skywalker::updateServiceChat(const QString& did)
{
    if (mBsky && mBsky->getSessionDid() == did)
        mBsky->setServiceChat(mUserSettings.getServiceChat(did));
}

void Skywalker::updateServiceVideoHost(const QString& did)
{
    if (mBsky && mBsky->getSessionDid() == did)
        mBsky->setServiceHostVideo(mUserSettings.getServiceVideoHost(did));
}

void Skywalker::updateServiceVideoDid(const QString& did)
{
    if (mBsky && mBsky->getSessionDid() == did)
        mBsky->setServiceDidVideo(mUserSettings.getServiceVideoDid(did));
}

ATProto::PostMaster* Skywalker::postMaster()
{
    if (!mPostMaster)
    {
        Q_ASSERT(mBsky);

        if (mBsky)
            mPostMaster = std::make_unique<ATProto::PostMaster>(*mBsky);
        else
            qWarning() << "Bsky client not yet created";
    }

    return mPostMaster.get();
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

    if (mBsky && mBsky->getSession())
        mUserSettings.saveSession(*mBsky->getSession());

    mUserSettings.sync();
    mSessionManager.saveTokens();
    mPostMaster = nullptr;

    stopTimelineAutoUpdate();
    stopRefreshTimers();
    mSessionManager.clear();
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
    mBookmarks = nullptr;
    mUserPreferences = ATProto::UserPreferences();
    mProfileMaster = nullptr;
    mEditUserPreferences = nullptr;
    mGlobalContentGroupListModel = nullptr;
    mTimelineModel.reset();
    mUserDid.clear();
    mUserProfile = {};
    mAnniversary.setFirstAppearance({});
    mLoggedOutVisibility = true;
    mFollowsActivityStore.clear();
    mMutedReposts.clear();
    mTimelineHide.clear();
    mContentFilterPolicies.clear();
    setUnreadNotificationCount(0);
    mBookmarksModel = nullptr;
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

    mSignOutInProgress = false;

    emit bskyClientDeleted();
}

}
