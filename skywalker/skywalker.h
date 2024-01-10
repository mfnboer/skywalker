// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "author_feed_model.h"
#include "author_list_model.h"
#include "bookmarks.h"
#include "bookmarks_model.h"
#include "content_group_list_model.h"
#include "edit_user_preferences.h"
#include "favorite_feeds.h"
#include "feed_list_model.h"
#include "invite_code.h"
#include "item_store.h"
#include "list_list_model.h"
#include "muted_words.h"
#include "notification_list_model.h"
#include "post_feed_model.h"
#include "post_thread_model.h"
#include "profile_store.h"
#include "search_post_feed_model.h"
#include "user_settings.h"
#include <atproto/lib/client.h>
#include <atproto/lib/profile_master.h>
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <QTimer>
#include <QtQmlIntegration>

namespace Skywalker {

class Skywalker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString VERSION MEMBER VERSION CONSTANT)
    Q_PROPERTY(const PostFeedModel* timelineModel READ getTimelineModel CONSTANT FINAL)
    Q_PROPERTY(NotificationListModel* notificationListModel READ getNotificationListModel CONSTANT FINAL)
    Q_PROPERTY(Bookmarks* bookmarks READ getBookmarks CONSTANT FINAL)
    Q_PROPERTY(MutedWords* mutedWords READ getMutedWords CONSTANT FINAL)
    Q_PROPERTY(bool autoUpdateTimelineInProgress READ isAutoUpdateTimelineInProgress NOTIFY autoUpdateTimeLineInProgressChanged FINAL)
    Q_PROPERTY(bool getTimelineInProgress READ isGetTimelineInProgress NOTIFY getTimeLineInProgressChanged FINAL)
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(bool getPostThreadInProgress READ isGetPostThreadInProgress NOTIFY getPostThreadInProgressChanged FINAL)
    Q_PROPERTY(bool getNotificationsInProgress READ isGetNotificationsInProgress NOTIFY getNotificationsInProgressChanged FINAL)
    Q_PROPERTY(bool getAuthorFeedInProgress READ isGetAuthorFeedInProgress NOTIFY getAuthorFeedInProgressChanged FINAL)
    Q_PROPERTY(bool getAuthorListInProgress READ isGetAuthorListInProgress NOTIFY getAuthorListInProgressChanged FINAL)
    Q_PROPERTY(bool getListListInProgress READ isGetListListInProgress NOTIFY getListListInProgressChanged FINAL)
    Q_PROPERTY(QString avatarUrl READ getAvatarUrl NOTIFY avatarUrlChanged FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(FavoriteFeeds* favoriteFeeds READ getFavoriteFeeds CONSTANT FINAL)
    QML_ELEMENT

public:
    static constexpr const char* VERSION = APP_VERSION;

    explicit Skywalker(QObject* parent = nullptr);
    ~Skywalker();

    Q_INVOKABLE void login(const QString user, QString password, const QString host);
    Q_INVOKABLE void resumeSession();
    Q_INVOKABLE void getUserProfileAndFollows();
    Q_INVOKABLE void getUserPreferences();
    Q_INVOKABLE void syncTimeline(int maxPages = 20);
    Q_INVOKABLE void getTimeline(int limit, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getTimelinePrepend(int autoGapFill = 0);
    Q_INVOKABLE void getTimelineForGap(int gapId, int autoGapFill = 0, bool userInitiated = false);
    Q_INVOKABLE void getTimelineNextPage(int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE void timelineMovementEnded(int firstVisibleIndex, int lastVisibleIndex);
    Q_INVOKABLE void getFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10);
    Q_INVOKABLE void getListFeed(int modelId, int limit = 50, int maxPages = 5, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getListFeedNextPage(int modelId, int maxPages = 5, int minEntries = 10);
    Q_INVOKABLE void getPostThread(const QString& uri);
    Q_INVOKABLE const PostThreadModel* getPostThreadModel(int id) const;
    Q_INVOKABLE void removePostThreadModel(int id);
    Q_INVOKABLE void updatePostIndexTimestamps();
    Q_INVOKABLE void getNotifications(int limit, bool updateSeen = false, const QString& cursor = {});
    Q_INVOKABLE void getNotificationsNextPage();
    Q_INVOKABLE void getBookmarksPage(bool clearModel = false);
    Q_INVOKABLE void getDetailedProfile(const QString& author);
    Q_INVOKABLE void clearAuthorFeed(int id);
    Q_INVOKABLE void getAuthorFeed(int id, int limit, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getAuthorFeedNextPage(int id, int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE int createAuthorFeedModel(const BasicProfile& author);
    Q_INVOKABLE const AuthorFeedModel* getAuthorFeedModel(int id) const;
    Q_INVOKABLE void removeAuthorFeedModel(int id);
    Q_INVOKABLE void getFeedGenerator(const QString& feedUri, bool viewPosts = false);
    Q_INVOKABLE int createSearchPostFeedModel();
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel(int id) const;
    Q_INVOKABLE void removeSearchPostFeedModel(int id);
    Q_INVOKABLE int createFeedListModel();
    Q_INVOKABLE FeedListModel* getFeedListModel(int id) const;
    Q_INVOKABLE void removeFeedListModel(int id);
    Q_INVOKABLE int createPostFeedModel(const GeneratorView& generatorView);
    Q_INVOKABLE int createPostFeedModel(const ListView& listView);
    Q_INVOKABLE PostFeedModel* getPostFeedModel(int id) const;
    Q_INVOKABLE void removePostFeedModel(int id);
    Q_INVOKABLE void getAuthorList(int id, int limit = 50, const QString& cursor = {});
    Q_INVOKABLE void getAuthorListNextPage(int id);
    Q_INVOKABLE int createAuthorListModel(AuthorListModel::Type type, const QString& atId);
    Q_INVOKABLE AuthorListModel* getAuthorListModel(int id) const;
    Q_INVOKABLE void removeAuthorListModel(int id);
    Q_INVOKABLE void getListList(int id, int limit = 50, int maxPages = 20, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getListListNextPage(int id, int limit = 50, int maxPages = 20, int minEntries = 10);
    Q_INVOKABLE int createListListModel(ListListModel::Type type, const QString& atId);
    Q_INVOKABLE ListListModel* getListListModel(int id) const;
    Q_INVOKABLE void removeListListModel(int id);
    Q_INVOKABLE QString getUserDid() const { return mUserDid; }
    Q_INVOKABLE BasicProfile getUser() const;
    Q_INVOKABLE void sharePost(const QString& postUri, const BasicProfile& author);
    Q_INVOKABLE void shareFeed(const GeneratorView& feed);
    Q_INVOKABLE void shareAuthor(const BasicProfile& author);
    Q_INVOKABLE void copyPostTextToClipboard(const QString& text);
    Q_INVOKABLE QEnums::ContentVisibility getContentVisibility(const QStringList& labelTexts) const;
    Q_INVOKABLE QString getContentWarning(const QStringList& labelTexts) const;
    Q_INVOKABLE const ContentGroupListModel* getContentGroupListModel();
    Q_INVOKABLE void saveContentFilterPreferences();
    Q_INVOKABLE EditUserPreferences* getEditUserPreferences();
    Q_INVOKABLE void saveUserPreferences();
    Q_INVOKABLE void saveFavoriteFeeds();

    // NOTE: destroys the previous model
    Q_INVOKABLE const BookmarksModel* createBookmarksModel();
    Q_INVOKABLE void deleteBookmarksModel();

    Q_INVOKABLE UserSettings* getUserSettings() { return &mUserSettings; }
    Q_INVOKABLE void showStatusMessage(const QString& msg, QEnums::StatusLevel level);
    Q_INVOKABLE bool sendAppToBackground();
    Q_INVOKABLE bool isSignedIn() const { return !mUserDid.isEmpty(); }
    Q_INVOKABLE void clearPassword();
    Q_INVOKABLE void signOut();

    void makeLocalModelChange(const std::function<void(LocalPostModelChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalAuthorModelChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalFeedModelChanges*)>& update);

    const PostFeedModel* getTimelineModel() const { return &mTimelineModel; }
    NotificationListModel* getNotificationListModel() { return &mNotificationListModel; }
    Bookmarks* getBookmarks() { return &mBookmarks; }
    MutedWords* getMutedWords() { return &mMutedWords; }
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
    const QString& getAvatarUrl() const { return mAvatarUrl; }
    void setAvatarUrl(const QString& avatarUrl);
    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);
    IndexedProfileStore& getUserFollows() { return mUserFollows; }
    const ContentFilter& getContentFilter() const { return mContentFilter; }
    ATProto::Client* getBskyClient() const { return mBsky.get(); }
    std::optional<QString> makeOptionalCursor(const QString& cursor) const;
    FavoriteFeeds* getFavoriteFeeds() { return &mFavoriteFeeds; }

signals:
    void loginOk();
    void loginFailed(QString error, QString host, QString handle);
    void resumeSessionOk();
    void resumeSessionExpired();
    void resumeSessionFailed(QString error);
    void timelineSyncOK(int index);
    void timelineSyncFailed();
    void timelineRefreshed(int prevTopPostIndex);
    void gapFilled(int gapEndIndex);
    void getUserProfileOK();
    void getUserProfileFailed(QString error);
    void getUserPreferencesOK();
    void getUserPreferencesFailed();
    void autoUpdateTimeLineInProgressChanged();
    void getTimeLineInProgressChanged();
    void getFeedInProgressChanged();
    void getNotificationsInProgressChanged();
    void sessionExpired(QString error);
    void statusMessage(QString msg, QEnums::StatusLevel level = QEnums::STATUS_LEVEL_INFO);
    void postThreadOk(int id, int postEntryIndex);
    void avatarUrlChanged();
    void unreadNotificationCountChanged();
    void getDetailedProfileOK(DetailedProfile);
    void getAuthorFeedInProgressChanged();
    void getAuthorListInProgressChanged();
    void getListListInProgressChanged();
    void getFeedGeneratorOK(GeneratorView generatorView, bool viewPosts);
    void getPostThreadInProgressChanged();
    void inviteCodes(InviteCodeList);
    void sharedTextReceived(QString text); // Shared from another app
    void sharedImageReceived(QString source, QString text); // Shared from another app
    void bskyClientDeleted();

private:
    void getUserProfileAndFollowsNextPage(const QString& cursor, int maxPages = 100);
    void getFollowsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getFollowersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getBlocksAuthorList(int limit, const QString& cursor, int modelId);
    void getMutesAuthorList(int limit, const QString& cursor, int modelId);
    void getLikesAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getRepostsAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void getListMembersAuthorList(const QString& atId, int limit, const QString& cursor, int modelId);
    void signalGetUserProfileOk(const ATProto::AppBskyActor::ProfileView& user);
    void syncTimeline(QDateTime tillTimestamp, int maxPages = 40, const QString& cursor = {});
    void finishTimelineSync(int index);
    void finishTimelineSyncFailed();
    void startRefreshTimers();
    void stopRefreshTimers();
    void refreshSession();
    void refreshNotificationCount();
    void updateUser(const QString& did, const QString& host, const QString& password);
    ATProto::ProfileMaster& getProfileMaster();
    void saveSession(const ATProto::ComATProtoServer::Session& session);
    bool getSession(QString& host, ATProto::ComATProtoServer::Session& session);
    void saveSyncTimestamp(int postIndex);
    QDateTime getSyncTimestamp() const;
    void shareImage(const QString& contentUri, const QString& text);
    void updateFavoriteFeeds();
    void saveUserPreferences(const ATProto::UserPreferences& prefs);
    void disableDebugLogging();
    void restoreDebugLogging();

    std::unique_ptr<ATProto::Client> mBsky;

    QString mAvatarUrl;
    QString mUserDid;
    bool mLoggedOutVisibility = true;
    IndexedProfileStore mUserFollows;
    ATProto::UserPreferences mUserPreferences;
    std::unique_ptr<ATProto::ProfileMaster> mProfileMaster;
    std::unique_ptr<EditUserPreferences> mEditUserPreferences;
    ContentFilter mContentFilter;
    std::unique_ptr<ContentGroupListModel> mContentGroupListModel;

    Bookmarks mBookmarks;
    BookmarksModel::Ptr mBookmarksModel;
    MutedWords mMutedWords;
    PostFeedModel mTimelineModel;

    bool mAutoUpdateTimelineInProgress = false;
    bool mGetTimelineInProgress = false;
    bool mGetFeedInProgress = false; // for feeds and listFeeds
    bool mGetPostThreadInProgress = false;
    bool mGetAuthorFeedInProgress = false;
    bool mGetAuthorListInProgress = false;
    bool mGetListListInProgress = false;
    bool mSignOutInProgress = false;

    QTimer mRefreshTimer;
    QTimer mRefreshNotificationTimer;

    // NOTE: update makeLocalModelChange() when you add models
    ItemStore<PostThreadModel::Ptr> mPostThreadModels;
    ItemStore<AuthorFeedModel::Ptr> mAuthorFeedModels;
    ItemStore<SearchPostFeedModel::Ptr> mSearchPostFeedModels;
    ItemStore<AuthorListModel::Ptr> mAuthorListModels;
    ItemStore<ListListModel::Ptr> mListListModels;
    ItemStore<FeedListModel::Ptr> mFeedListModels;
    ItemStore<PostFeedModel::Ptr> mPostFeedModels;
    NotificationListModel mNotificationListModel;

    bool mGetNotificationsInProgress = false;
    int mUnreadNotificationCount = 0;

    FavoriteFeeds mFavoriteFeeds;
    UserSettings mUserSettings;
    bool mDebugLogging = false;
};

}
