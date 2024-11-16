// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "anniversary.h"
#include "author_feed_model.h"
#include "author_list_model.h"
#include "bookmarks.h"
#include "bookmarks_model.h"
#include "content_group_list_model.h"
#include "draft_posts_migration.h"
#include "draft_posts_model.h"
#include "edit_user_preferences.h"
#include "favorite_feeds.h"
#include "feed_list_model.h"
#include "hashtag_index.h"
#include "item_store.h"
#include "labeler.h"
#include "list_list_model.h"
#include "muted_words.h"
#include "notification_list_model.h"
#include "post_feed_model.h"
#include "post_thread_model.h"
#include "profile_store.h"
#include "search_post_feed_model.h"
#include "starter_pack_list_model.h"
#include "user_settings.h"
#include <atproto/lib/client.h>
#include <atproto/lib/plc_directory_client.h>
#include <atproto/lib/profile_master.h>
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QTimer>
#include <QtQmlIntegration>

namespace Skywalker {

class Chat;
class FocusHashtags;

class Skywalker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString VERSION MEMBER VERSION CONSTANT)
    Q_PROPERTY(int TIMELINE_PREPEND_PAGE_SIZE MEMBER TIMELINE_PREPEND_PAGE_SIZE CONSTANT)
    Q_PROPERTY(const PostFeedModel* timelineModel READ getTimelineModel CONSTANT FINAL)
    Q_PROPERTY(NotificationListModel* notificationListModel READ getNotificationListModel CONSTANT FINAL)
    Q_PROPERTY(Chat* chat READ getChat CONSTANT FINAL)
    Q_PROPERTY(Bookmarks* bookmarks READ getBookmarks CONSTANT FINAL)
    Q_PROPERTY(MutedWords* mutedWords READ getMutedWords CONSTANT FINAL)
    Q_PROPERTY(FocusHashtags* focusHashtags READ getFocusHashtags CONSTANT FINAL)
    Q_PROPERTY(bool autoUpdateTimelineInProgress READ isAutoUpdateTimelineInProgress NOTIFY autoUpdateTimeLineInProgressChanged FINAL)
    Q_PROPERTY(bool getTimelineInProgress READ isGetTimelineInProgress NOTIFY getTimeLineInProgressChanged FINAL)
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(bool getPostThreadInProgress READ isGetPostThreadInProgress NOTIFY getPostThreadInProgressChanged FINAL)
    Q_PROPERTY(bool getNotificationsInProgress READ isGetNotificationsInProgress NOTIFY getNotificationsInProgressChanged FINAL)
    Q_PROPERTY(bool getAuthorFeedInProgress READ isGetAuthorFeedInProgress NOTIFY getAuthorFeedInProgressChanged FINAL)
    Q_PROPERTY(bool getAuthorListInProgress READ isGetAuthorListInProgress NOTIFY getAuthorListInProgressChanged FINAL)
    Q_PROPERTY(bool getListListInProgress READ isGetListListInProgress NOTIFY getListListInProgressChanged FINAL)
    Q_PROPERTY(bool getStarterPackListInProgress READ isGetStarterPackListInProgress NOTIFY getStarterPackListInProgressChanged FINAL)
    Q_PROPERTY(BasicProfile user READ getUser NOTIFY userChanged FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(FavoriteFeeds* favoriteFeeds READ getFavoriteFeeds CONSTANT FINAL)
    QML_ELEMENT

public:
    static constexpr const char* VERSION = APP_VERSION;
    static constexpr int TIMELINE_PREPEND_PAGE_SIZE = 20;

    explicit Skywalker(QObject* parent = nullptr);
    ~Skywalker();

    Q_INVOKABLE void login(const QString user, QString password, const QString host, bool rememberPassword, const QString authFactorToken);
    Q_INVOKABLE bool autoLogin();
    Q_INVOKABLE bool resumeSession(bool retry = false);
    Q_INVOKABLE void deleteSession();
    Q_INVOKABLE void switchUser(const QString& did);
    Q_INVOKABLE void getUserProfileAndFollows();
    Q_INVOKABLE void getUserPreferences();
    Q_INVOKABLE void dataMigration();
    Q_INVOKABLE void syncTimeline(int maxPages = 20);
    Q_INVOKABLE void startTimelineAutoUpdate();
    Q_INVOKABLE void stopTimelineAutoUpdate();
    Q_INVOKABLE void getTimeline(int limit, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
                void getTimelinePrepend(int autoGapFill = 0, int pageSize = TIMELINE_PREPEND_PAGE_SIZE);
    Q_INVOKABLE void getTimelineForGap(int gapId, int autoGapFill = 0, bool userInitiated = false);
    Q_INVOKABLE void getTimelineNextPage(int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE void updateTimeline(int autoGapFill, int pageSize);
    Q_INVOKABLE void timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex);
    Q_INVOKABLE void getFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10);
    Q_INVOKABLE void getListFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getListFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10);
    Q_INVOKABLE void getQuotesFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getQuotesFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10);
    Q_INVOKABLE void getPostThread(const QString& uri, int modelId = -1);
    Q_INVOKABLE PostThreadModel* getPostThreadModel(int id) const;
    Q_INVOKABLE void removePostThreadModel(int id);
    Q_INVOKABLE void updateNotificationPreferences(bool priority);
    Q_INVOKABLE void getNotifications(int limit = 25, bool updateSeen = false, const QString& cursor = {});
    Q_INVOKABLE void getNotificationsNextPage();
    Q_INVOKABLE void getBookmarksPage(bool clearModel = false);
    Q_INVOKABLE void getDetailedProfile(const QString& author);

    // If avatar is a "image://", then the profile takes ownership of the image
    Q_INVOKABLE void updateUserProfile(const QString& displayName, const QString& description,
                                       const QString& avatar);

    Q_INVOKABLE void clearAuthorFeed(int id);
    Q_INVOKABLE void getAuthorFeed(int id, int limit, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getAuthorFeedNextPage(int id, int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE void getAuthorLikes(int id, int limit = 50, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getAuthorLikesNextPage(int id, int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE int createAuthorFeedModel(const DetailedProfile& author, QEnums::AuthorFeedFilter filter = QEnums::AUTHOR_FEED_FILTER_POSTS);
    Q_INVOKABLE const AuthorFeedModel* getAuthorFeedModel(int id) const;
    Q_INVOKABLE void removeAuthorFeedModel(int id);
    Q_INVOKABLE void getFeedGenerator(const QString& feedUri, bool viewPosts = false);
    Q_INVOKABLE void getStarterPackView(const QString& starterPackUri);
    Q_INVOKABLE int createSearchPostFeedModel();
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel(int id) const;
    Q_INVOKABLE void removeSearchPostFeedModel(int id);
    Q_INVOKABLE void getAuthorFeedList(const QString& did, int id, const QString& cursor = {});
    Q_INVOKABLE void getAuthorFeedListNextPage(const QString& did, int id);
    Q_INVOKABLE int createFeedListModel();
    Q_INVOKABLE FeedListModel* getFeedListModel(int id) const;
    Q_INVOKABLE void removeFeedListModel(int id);
    Q_INVOKABLE void getAuthorStarterPackList(const QString& did, int id, const QString& cursor = {});
    Q_INVOKABLE void getAuthorStarterPackListNextPage(const QString& did, int id);
    Q_INVOKABLE int createStarterPackListModel();
    Q_INVOKABLE StarterPackListModel* getStarterPackListModel(int id) const;
    Q_INVOKABLE void removeStarterPackListModel(int id);
    Q_INVOKABLE int createPostFeedModel(const GeneratorView& generatorView);
    Q_INVOKABLE int createPostFeedModel(const ListViewBasic& listView);
    Q_INVOKABLE int createQuotePostFeedModel(const QString& quoteUri);
    Q_INVOKABLE PostFeedModel* getPostFeedModel(int id) const;
    Q_INVOKABLE void removePostFeedModel(int id);
    Q_INVOKABLE void getAuthorList(int id, int limit = 50, const QString& cursor = {});
    Q_INVOKABLE void getAuthorListNextPage(int id);
    Q_INVOKABLE int createAuthorListModel(AuthorListModel::Type type, const QString& atId);
    Q_INVOKABLE AuthorListModel* getAuthorListModel(int id) const;
    Q_INVOKABLE void removeAuthorListModel(int id);
    Q_INVOKABLE void getListList(int id, int limit = 50, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getListListNextPage(int id, int limit = 50, int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE int createListListModel(ListListModel::Type type, ListListModel::Purpose purpose, const QString& atId);
    Q_INVOKABLE ListListModel* getListListModel(int id) const;
    Q_INVOKABLE void removeListListModel(int id);
    Q_INVOKABLE QString getUserDid() const { return mUserDid; }
    Q_INVOKABLE Profile getUserProfile() const { return mUserProfile; }
    Q_INVOKABLE BasicProfile getUser() const;
    Q_INVOKABLE void sharePost(const QString& postUri);
    Q_INVOKABLE void shareFeed(const GeneratorView& feed);
    Q_INVOKABLE void shareList(const ListView& list);
    Q_INVOKABLE void shareAuthor(const BasicProfile& author);
    Q_INVOKABLE void copyPostTextToClipboard(const QString& text);
    Q_INVOKABLE void copyToClipboard(const QString& text);
    Q_INVOKABLE ContentGroup getContentGroup(const QString& did, const QString& labelId) const;
    Q_INVOKABLE QEnums::ContentVisibility getContentVisibility(const ContentLabelList& contetLabels) const;
    Q_INVOKABLE QString getContentWarning(const ContentLabelList& contentLabels) const;
    Q_INVOKABLE const ContentGroupListModel* getGlobalContentGroupListModel();
    Q_INVOKABLE int createContentGroupListModel(const QString& did, const LabelerPolicies& policies);
    Q_INVOKABLE ContentGroupListModel* getContentGroupListModel(int id) const;
    Q_INVOKABLE void removeContentGroupListModel(int id);
    Q_INVOKABLE void saveGlobalContentFilterPreferences();
    Q_INVOKABLE void saveContentFilterPreferences(const ContentGroupListModel* model);
    Q_INVOKABLE ContentFilter* getContentFilter() { return &mContentFilter; }
    Q_INVOKABLE Anniversary* getAnniversary() { return &mAnniversary; }
    Q_INVOKABLE EditUserPreferences* getEditUserPreferences();
    Q_INVOKABLE void saveUserPreferences();
    Q_INVOKABLE void saveFavoriteFeeds();
    Q_INVOKABLE void loadBookmarks();
    Q_INVOKABLE void loadMutedWords();
    Q_INVOKABLE void saveMutedWords(std::function<void()> okCb = {});
    Q_INVOKABLE void loadHashtags();
    void saveHashtags();

    // NOTE: destroys the previous model
    Q_INVOKABLE const BookmarksModel* createBookmarksModel();
    Q_INVOKABLE void deleteBookmarksModel();

    Q_INVOKABLE UserSettings* getUserSettings() { return &mUserSettings; }
    Q_INVOKABLE void showStatusMessage(const QString& msg, QEnums::StatusLevel level);

    // TODO: refactor to separate App Utils class
    Q_INVOKABLE bool sendAppToBackground();
    Q_INVOKABLE void setNavigationBarColor(QColor color);

    Q_INVOKABLE bool isSignedIn() const { return !mUserDid.isEmpty(); }
    Q_INVOKABLE void signOut();

    void makeLocalModelChange(const std::function<void(LocalProfileChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalPostModelChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalAuthorModelChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalFeedModelChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalListModelChanges*)>& update);

    const PostFeedModel* getTimelineModel() const { return &mTimelineModel; }
    NotificationListModel* getNotificationListModel() { return &mNotificationListModel; }
    Chat* getChat();
    Bookmarks* getBookmarks() { return &mBookmarks; }
    MutedWords* getMutedWords() { return &mMutedWords; }
    FocusHashtags* getFocusHashtags() { return mFocusHashtags.get(); }
    void setAutoUpdateTimelineInProgress(bool inProgress);
    bool isAutoUpdateTimelineInProgress() const { return mAutoUpdateTimelineInProgress; }
    void setGetTimelineInProgress(bool inProgress);
    bool isGetTimelineInProgress() const { return mGetTimelineInProgress; }
    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }
    void setGetPostThreadInProgress(bool inProgress);
    bool isGetPostThreadInProgress() const { return mGetPostThreadInProgress; }
    void setGetNotificationsInProgress(bool inProgress);
    bool isGetNotificationsInProgress() const { return mGetNotificationsInProgress; }
    void setGetAuthorFeedInProgress(bool inProgress);
    bool isGetAuthorFeedInProgress() const { return mGetAuthorFeedInProgress; }
    void setGetAuthorListInProgress(bool inProgress);
    bool isGetAuthorListInProgress() const { return mGetAuthorListInProgress; }
    void setGetListListInProgress(bool inProgress);
    bool isGetListListInProgress() const { return mGetListListInProgress; }
    void setGetStarterPackListInProgress(bool inProgress);
    bool isGetStarterPackListInProgress() const { return mGetStarterPackListInProgress; }
    const QString getAvatarUrl() const { return mUserProfile.getAvatarUrl(); }
    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);
    void addToUnreadNotificationCount(int addUnread);
    IndexedProfileStore& getUserFollows() { return mUserFollows; }
    ProfileListItemStore& getMutedReposts() { return mMutedReposts; }
    ATProto::Client* getBskyClient() const { return mBsky.get(); }
    ATProto::PlcDirectoryClient& getPlcDirectory() { return mPlcDirectory; }
    HashtagIndex& getUserHashtags() { return mUserHashtags; }
    HashtagIndex& getSeenHashtags() { return mSeenHashtags; }
    FavoriteFeeds* getFavoriteFeeds() { return &mFavoriteFeeds; }
    DraftPostsModel::Ptr createDraftPostsModel();

signals:
    void loginOk();
    void loginFailed(QString error, QString msg, QString host, QString handle, QString password);
    void resumeSessionOk();
    void resumeSessionFailed(QString error);
    void sessionDeleted();
    void timelineSyncOK(int index);
    void timelineSyncFailed();
    void timelineRefreshed(int prevTopPostIndex);
    void gapFilled(int gapEndIndex);
    void getUserProfileOK();
    void getUserProfileFailed(QString error);
    void getUserPreferencesOK();
    void getUserPreferencesFailed(QString error);
    void dataMigrationStatus(QString status);
    void dataMigrationDone();
    void autoUpdateTimeLineInProgressChanged();
    void getTimeLineInProgressChanged();
    void getFeedInProgressChanged();
    void getNotificationsInProgressChanged();
    void sessionExpired(QString error);
    void getAuthorFeedOk(int modelId);
    void getAuthorFeedFailed(int modelId, QString error, QString msg);
    void statusMessage(QString msg, QEnums::StatusLevel level = QEnums::STATUS_LEVEL_INFO);
    void postThreadOk(int id, int postEntryIndex);
    void userChanged();
    void unreadNotificationCountChanged();
    void getDetailedProfileOK(DetailedProfile);
    void getAuthorFeedInProgressChanged();
    void getAuthorListInProgressChanged();
    void getListListInProgressChanged();
    void getStarterPackListInProgressChanged();
    void getFeedGeneratorOK(GeneratorView generatorView, bool viewPosts);
    void getStarterPackViewOk(StarterPackView starterPack);
    void getPostThreadInProgressChanged();
    void sharedTextReceived(QString text); // Shared from another app
    void sharedImageReceived(QString source, QString gifTempFileName, QString text); // Shared from another app
    void sharedVideoReceived(QUrl url, QString text); // Shared from another app
    void sharedDmTextReceived(QString text); // Shared from another app
    void showNotifications(); // Action received from clicking an app notification
    void showDirectMessages(); // Action received from clicking an app notification
    void bskyClientDeleted();
    void anniversary();
    void oldestUnreadNotificationIndex(int index);

private:
    void getUserProfileAndFollowsNextPage(const QString& cursor, int maxPages = 100);
    void getLabelersAuthorList(int modelId);
    void getFollowsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getKnownFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getBlocksAuthorList(int limit, const QString& cursor, int modelId);
    void getMutesAuthorList(int limit, const QString& cursor, int modelId);
    void getSuggestionsAuthorList(int limit, const QString& cursor, int modelId);
    void getLikesAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getRepostsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getListMembersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getListListAll(const QString& atId, int limit, int maxPages, int minEntries, const QString& cursor, int modelId);
    void getListListBlocks(int limit, int maxPages, int minEntries, const QString& cursor, int modelId);
    void getListListMutes(int limit, int maxPages, int minEntries, const QString& cursor, int modelId);
    void signalGetUserProfileOk(ATProto::AppBskyActor::ProfileView::SharedPtr user);
    void syncTimeline(QDateTime tillTimestamp, int maxPages = 40, const QString& cursor = {});
    void finishTimelineSync(int index);
    void finishTimelineSyncFailed();
    void updatePostIndexedSecondsAgo();
    void startRefreshTimers();
    void stopRefreshTimers();
    void refreshSession(const std::function<void()>& cbOk = {});
    void refreshNotificationCount();
    void updateUser(const QString& did, const QString& host);
    ATProto::ProfileMaster& getProfileMaster();
    void saveSession(const ATProto::ComATProtoServer::Session& session);
    bool getSavedSession(QString& host, ATProto::ComATProtoServer::Session& session);
    void saveSyncTimestamp(int postIndex);
    QDateTime getSyncTimestamp() const;
    void shareImage(const QString& contentUri, const QString& text);
    void shareVideo(const QString& contentUri, const QString& text);
    void updateFavoriteFeeds();
    void saveUserPreferences(const ATProto::UserPreferences& prefs, std::function<void()> okCb = nullptr);
    void loadMutedReposts(int maxPages = 10, const QString& cursor = {});
    void initLabelers();
    void loadLabelSettings();
    void removeLabelerSubscriptions(const std::unordered_set<QString>& dids);
    void disableDebugLogging();
    void restoreDebugLogging();
    void handleAppStateChange(Qt::ApplicationState state);
    void pauseApp();
    void resumeApp();
    void migrateDraftPosts();
    void checkAnniversary();

    ATProto::Client::Ptr mBsky;
    ATProto::PlcDirectoryClient mPlcDirectory;

    QString mAvatarUrl;
    QString mUserDid;
    Profile mUserProfile;

    bool mLoggedOutVisibility = true;
    IndexedProfileStore mUserFollows;
    ProfileListItemStore mMutedReposts;
    ATProto::UserPreferences mUserPreferences;
    std::unique_ptr<ATProto::ProfileMaster> mProfileMaster;
    std::unique_ptr<EditUserPreferences> mEditUserPreferences;
    UserSettings mUserSettings;
    ContentFilter mContentFilter;
    ContentFilterShowAll mContentFilterShowAll;
    ContentGroupListModel::Ptr mGlobalContentGroupListModel;

    Bookmarks mBookmarks;
    BookmarksModel::Ptr mBookmarksModel;
    MutedWords mMutedWords;
    MutedWordsNoMutes mMutedWordsNoMutes;
    std::unique_ptr<FocusHashtags> mFocusHashtags;

    bool mAutoUpdateTimelineInProgress = false;
    bool mGetTimelineInProgress = false;
    bool mGetFeedInProgress = false; // for feeds and listFeeds and feedList and starterPackList
    bool mGetPostThreadInProgress = false;
    bool mGetAuthorFeedInProgress = false;
    bool mGetAuthorListInProgress = false;
    bool mGetListListInProgress = false;
    bool mGetStarterPackListInProgress = false;
    bool mSignOutInProgress = false;

    QTimer mRefreshTimer;
    QTimer mRefreshNotificationTimer;
    QTimer mTimelineUpdateTimer;
    bool mTimelineUpdatePaused = false;

    // NOTE: update makeLocalModelChange() when you add models
    ItemStore<PostThreadModel::Ptr> mPostThreadModels;
    ItemStore<AuthorFeedModel::Ptr> mAuthorFeedModels;
    ItemStore<SearchPostFeedModel::Ptr> mSearchPostFeedModels;
    ItemStore<AuthorListModel::Ptr> mAuthorListModels;
    ItemStore<ListListModel::Ptr> mListListModels;
    ItemStore<FeedListModel::Ptr> mFeedListModels;
    ItemStore<StarterPackListModel::Ptr> mStarterPackListModels;
    ItemStore<PostFeedModel::Ptr> mPostFeedModels;
    ItemStore<ContentGroupListModel::Ptr> mContentGroupListModels;
    NotificationListModel mNotificationListModel;
    std::unique_ptr<Chat> mChat;

    bool mGetNotificationsInProgress = false;
    int mUnreadNotificationCount = 0;

    HashtagIndex mUserHashtags;
    HashtagIndex mSeenHashtags;
    FavoriteFeeds mFavoriteFeeds;
    Anniversary mAnniversary;
    std::unique_ptr<DraftPostsMigration> mDraftPostsMigration;
    PostFeedModel mTimelineModel;
    bool mTimelineSynced = false;
    bool mDebugLogging = false;
};

}
