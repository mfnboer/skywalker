// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "password_encryption.h"
#include "profile.h"
#include "search_feed.h"
#include "uri_with_expiry.h"
#include <atproto/lib/client.h>
#include <QObject>
#include <QSettings>

namespace Skywalker {

class IUserSettings
{
public:
    virtual ~IUserSettings() = default;
    virtual QDate getAnniversaryNoticeDate(const QString& did) const = 0;
    virtual void setAnniversaryNoticeDate(const QString& did, QDate date) = 0;
};

class UserSettings : public QObject, public IUserSettings
{
    Q_OBJECT
    Q_PROPERTY(QString backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged FINAL)
    Q_PROPERTY(QString accentColor READ getAccentColor WRITE setAccentColor NOTIFY accentColorChanged FINAL)
    Q_PROPERTY(QString linkColor READ getLinkColor WRITE setLinkColor NOTIFY linkColorChanged FINAL)
    Q_PROPERTY(QEnums::ThreadStyle threadStyle READ getThreadStyle WRITE setThreadStyle NOTIFY threadStyleChanged FINAL)
    Q_PROPERTY(QString threadColor READ getThreadColor WRITE setThreadColor NOTIFY threadColorChanged FINAL)
    Q_PROPERTY(QEnums::FavoritesBarPosition favoritesBarPosition READ getFavoritesBarPosition WRITE setFavoritesBarPosition NOTIFY favoritesBarPositionChanged FINAL)
    Q_PROPERTY(bool giantEmojis READ getGiantEmojis WRITE setGiantEmojis NOTIFY giantEmojisChanged FINAL)
    Q_PROPERTY(bool songlinkEnabled READ getSonglinkEnabled WRITE setSonglinkEnabled NOTIFY songlinkEnabledChanged FINAL)
    Q_PROPERTY(bool showFollowsActiveStatus READ getShowFollowsActiveStatus WRITE setShowFollowsActiveStatus NOTIFY showFollowsActiveStatusChanged FINAL)
    Q_PROPERTY(bool landscapeSideBar READ getLandscapeSideBar WRITE setLandscapeSideBar NOTIFY landscapeSideBarChanged FINAL)
    Q_PROPERTY(bool gifAutoPlay READ getGifAutoPlay WRITE setGifAutoPlay NOTIFY gifAutoPlayChanged FINAL)
    Q_PROPERTY(bool videoSound READ getVideoSound WRITE setVideoSound NOTIFY videoSoundChanged FINAL)
    Q_PROPERTY(bool videoAutoPlay READ getVideoAutoPlay WRITE setVideoAutoPlay NOTIFY videoAutoPlayChanged FINAL)
    Q_PROPERTY(bool videoAutoLoad READ getVideoAutoLoad WRITE setVideoAutoLoad NOTIFY videoAutoLoadChanged FINAL)
    Q_PROPERTY(bool videoLoopPlay READ getVideoLoopPlay WRITE setVideoLoopPlay NOTIFY videoLoopPlayChanged FINAL)
    Q_PROPERTY(bool videoStreamingEnabled READ getVideoStreamingEnabled WRITE setVideoStreamingEnabled NOTIFY videoStreamingEnabledChanged FINAL)
    Q_PROPERTY(QEnums::VideoQuality videoQuality READ getVideoQuality WRITE setVideoQuality NOTIFY videoQualityChanged FINAL)
    Q_PROPERTY(bool floatingNavigationButtons READ getFloatingNavigationButtons WRITE setFloatingNavigationButtons NOTIFY floatingNavigationButtonsChanged FINAL)
    Q_PROPERTY(QEnums::Script scriptRecognition READ getScriptRecognition WRITE setScriptRecognition NOTIFY scriptRecognitionChanged FINAL)
    Q_PROPERTY(bool showTrendingTopics READ getShowTrendingTopics WRITE setShowTrendingTopics NOTIFY showTrendingTopicsChanged FINAL)
    Q_PROPERTY(bool showSuggestedUsers READ getShowSuggestedUsers WRITE setShowSuggestedUsers NOTIFY showSuggestedUsersChanged FINAL)
    Q_PROPERTY(UriWithExpirySet* blocksWithExpiry READ getBlocksWithExpiry NOTIFY blocksWithExpiryChanged FINAL)
    Q_PROPERTY(UriWithExpirySet* mutesWithExpiry READ getMutesWithExpiry NOTIFY mutesWithExpiryChanged FINAL)

public:
    void reset();

    Q_INVOKABLE  static int getActiveOnlineIntervalMins();

    Q_INVOKABLE static QEnums::DisplayMode getActiveDisplayMode() { return sActiveDisplayMode; }
    Q_INVOKABLE void setActiveDisplayMode(QEnums::DisplayMode mode);

    static QString getDefaultBackgroundColor() { return sDefaultBackgroundColor; }
    Q_INVOKABLE static void setDefaultBackgroundColor(const QString& color) { sDefaultBackgroundColor = color; }

    Q_INVOKABLE void resetLinkColor();
    QString getLinkColor() const;
    Q_INVOKABLE void setLinkColor(const QString& color);
    static QString getCurrentLinkColor() { return sCurrentLinkColor; }
    Q_INVOKABLE static void setCurrentLinkColor(const QString& color) { sCurrentLinkColor = color; }

    explicit UserSettings(QObject* parent = nullptr);
    explicit UserSettings(const QString& fileName, QObject* parent = nullptr);

    Q_INVOKABLE QList<BasicProfile> getUserList() const;

    // Get the user list with a surrogate profile added for adding a new user account
    Q_INVOKABLE QList<BasicProfile> getUserListWithAddAccount() const;

    QStringList getUserDidList() const;

    Q_INVOKABLE BasicProfile getUser(const QString& did) const;

    void setActiveUserDid(const QString& did);
    Q_INVOKABLE QString getActiveUserDid() const;

    void addUser(const QString& did, const QString& host);
    Q_INVOKABLE void removeUser(const QString& did);

    Q_INVOKABLE QString getHost(const QString& did) const;

    Q_INVOKABLE void setRememberPassword(const QString& did, bool enable);
    Q_INVOKABLE bool getRememberPassword(const QString& did) const;

    Q_INVOKABLE void savePassword(const QString& did, const QString& password);
    Q_INVOKABLE QString getPassword(const QString& did) const;

    QString getHandle(const QString& did) const;

    void saveDisplayName(const QString& did, const QString& displayName);
    QString getDisplayName(const QString& did) const;

    void saveAvatar(const QString& did, const QString& avatar);
    QString getAvatar(const QString& did) const;

    void saveSession(const ATProto::ComATProtoServer::Session& session);
    ATProto::ComATProtoServer::Session getSession(const QString& did) const;

    void clearTokens(const QString& did);
    void clearCredentials(const QString& did);

    void saveSyncTimestamp(const QString& did, QDateTime timestamp);
    QDateTime getSyncTimestamp(const QString& did) const;

    void saveSyncCid(const QString& did, const QString& cid);
    QString getSyncCid(const QString& did) const;

    void saveSyncOffsetY(const QString& did, int offsetY);
    int getSyncOffsetY(const QString& did) const;

    void setTimelineViews(const QString& did, const QJsonObject postFilters);
    QJsonObject getTimelineViews(const QString& did) const;

    // Currently these feeds are feeds pinned as favorites
    // [FAVORITES]
    void saveFeedSyncTimestamp(const QString& did, const QString& feedUri, QDateTime timestamp);
    QDateTime getFeedSyncTimestamp(const QString& did, const QString& feedUri) const;

    void saveFeedSyncCid(const QString& did, const QString& feedUri, const QString& cid);
    QString getFeedSyncCid(const QString& did, const QString& feedUri) const;

    void saveFeedSyncOffsetY(const QString& did, const QString& feedUri, int offsetY);
    int getFeedSyncOffsetY(const QString& did, const QString& feedUri) const;

    void addSyncFeed(const QString& did, const QString& feedUri);
    void removeSyncFeed(const QString& did, const QString& feedUri);
    const std::unordered_set<QString>& getSyncFeeds(const QString& did) const;
    Q_INVOKABLE bool mustSyncFeed(const QString& did, const QString& feedUri) const;

    Q_INVOKABLE void setFeedViewMode(const QString& did, const QString& feedUri, QEnums::ContentMode mode);
    Q_INVOKABLE QEnums::ContentMode getFeedViewMode(const QString& did, const QString& feedUri);
    QStringList getFeedViewModeUris(const QString& did) const;

    Q_INVOKABLE void setFeedHideReplies(const QString& did, const QString& feedUri, bool hide);
    Q_INVOKABLE bool getFeedHideReplies(const QString& did, const QString& feedUri) const;
    QStringList getFeedHideRepliesUris(const QString& did) const;

    Q_INVOKABLE void setFeedHideFollowing(const QString& did, const QString& feedUri, bool hide);
    Q_INVOKABLE bool getFeedHideFollowing(const QString& did, const QString& feedUri) const;
    QStringList getFeedHideFollowingUris(const QString& did) const;

    void setUserOrderedPinnedFeeds(const QString& did, QStringList favoriteKeys);
    QStringList getUserOrderedPinnedFeed(const QString& did) const;
    // [FAVORITES]

    Q_INVOKABLE void updateLastSignInTimestamp(const QString& did);
    Q_INVOKABLE QDateTime getLastSignInTimestamp(const QString& did) const;

    // For the home feed, the URI is "home"
    // For search feeds, the URI is the search name
    Q_INVOKABLE void setLastViewedFeed(const QString& did, const QString& uri);
    Q_INVOKABLE QString getLastViewedFeed(const QString& did) const;

    void setHideLists(const QString& did, const QStringList& listUris);
    QStringList getHideLists(const QString& did) const;

    void setBookmarks(const QString& did, const QStringList& bookmarks);
    QStringList getBookmarks(const QString& did) const;

    void setBookmarksMigrationAttempts(const QString& did, int attempts);
    int getBookmarksMigrationAttempts(const QString& did) const;

    // Legacy
    QStringList getMutedWords(const QString& did) const;
    void removeMutedWords(const QString& did);

    UriWithExpirySet* getBlocksWithExpiry();
    UriWithExpirySet* getBlocksWithExpiry(const QString& did);
    void addBlockWithExpiry(const QString& did, const UriWithExpiry& block);
    bool removeBlockWithExpiry(const QString& did, const QString& blockUri);

    UriWithExpirySet* getMutesWithExpiry();
    UriWithExpirySet* getMutesWithExpiry(const QString& did);
    void addMuteWithExpiry(const QString& did, const UriWithExpiry& mute);
    bool removeMuteWithExpiry(const QString& did, const QString& muteDid);

    Q_INVOKABLE void setDisplayMode(QEnums::DisplayMode displayMode);
    Q_INVOKABLE QEnums::DisplayMode getDisplayMode() const;

    Q_INVOKABLE void resetBackgroundColor();
    void setBackgroundColor(const QString& color);
    QString getBackgroundColor() const;

    Q_INVOKABLE void resetAccentColor();
    void setAccentColor(const QString& color);
    QString getAccentColor() const;

    void setThreadStyle(QEnums::ThreadStyle threadStyle);
    QEnums::ThreadStyle getThreadStyle() const;

    Q_INVOKABLE void resetThreadColor();
    void setThreadColor(const QString& color);
    QString getThreadColor() const;

    void setFavoritesBarPosition(QEnums::FavoritesBarPosition position);
    QEnums::FavoritesBarPosition getFavoritesBarPosition() const;

    Q_INVOKABLE void setPostButtonRelativeX(double x);
    Q_INVOKABLE double getPostButtonRelativeX() const;

    void setLandscapeSideBar(bool enabled);
    bool getLandscapeSideBar() const;

    void setGifAutoPlay(bool autoPlay);
    bool getGifAutoPlay() const;

    void setVideoStreamingEnabled(bool enabled);
    bool getVideoStreamingEnabled() const;

    void setVideoQuality(QEnums::VideoQuality quality);
    QEnums::VideoQuality getVideoQuality() const;

    void setVideoAutoPlay(bool autoPlay);
    bool getVideoAutoPlay() const;

    void setVideoAutoLoad(bool autoLoad);
    bool getVideoAutoLoad() const;

    void setVideoSound(bool on);
    bool getVideoSound() const;

    void setVideoLoopPlay(bool loop);
    bool getVideoLoopPlay() const;

    void setGiantEmojis(bool giantEmojis);
    bool getGiantEmojis() const;

    void setSonglinkEnabled(bool enabled);
    bool getSonglinkEnabled() const;

    void setFloatingNavigationButtons(bool floating);
    bool getFloatingNavigationButtons() const;

    void setShowFollowsActiveStatus(bool show);
    bool getShowFollowsActiveStatus() const;

    Q_INVOKABLE void setRequireAltText(const QString& did, bool require);
    Q_INVOKABLE bool getRequireAltText(const QString& did) const;

    void setScriptRecognition(QEnums::Script script);
    QEnums::Script getScriptRecognition() const;

    Q_INVOKABLE void setAutoLinkCard(bool autoLinkCard);
    Q_INVOKABLE bool getAutoLinkCard() const;

    Q_INVOKABLE void setThreadAutoNumber(bool autoNumber);
    Q_INVOKABLE bool getThreadAutoNumber() const;

    Q_INVOKABLE void setThreadPrefix(QString prefix);
    Q_INVOKABLE QString getThreadPrefix() const;

    Q_INVOKABLE void setThreadAutoSplit(bool autoSplit);
    Q_INVOKABLE bool getThreadAutoSplit() const;

    Q_INVOKABLE QString getMutedRepostsListUri(const QString& did) const;

    void setUserHashtags(const QString& did, const QStringList& hashtags);
    QStringList getUserHashtags(const QString& did) const;

    void setSeenHashtags(const QStringList& hashtags);
    QStringList getSeenHashtags() const;

    void setOfflineUnread(const QString& did, int unread);
    int getOfflineUnread(const QString& did) const;

    void setOfflineMessageCheckTimestamp(QDateTime timestamp);
    QDateTime getOfflineMessageCheckTimestamp() const;

    void setOffLineChatCheckRev(const QString& did, const QString& rev);
    QString getOffLineChatCheckRev(const QString& did) const;

    void setCheckOfflineChat(const QString& did, bool check);
    bool mustCheckOfflineChat(const QString& did) const;

    void resetNextNotificationId();
    int getNextNotificationId();

    Q_INVOKABLE void setNotificationsWifiOnly(bool enable);
    Q_INVOKABLE bool getNotificationsWifiOnly() const;

    Q_INVOKABLE bool getShowQuotesWithBlockedPost(const QString& did) const;
    Q_INVOKABLE void setShowQuotesWithBlockedPost(const QString& did, bool show);

    Q_INVOKABLE bool getShowFollowedReposts(const QString& did) const;
    Q_INVOKABLE void setShowFollowedReposts(const QString& did, bool show);

    Q_INVOKABLE bool getShowSelfReposts(const QString& did) const;
    Q_INVOKABLE void setShowSelfReposts(const QString& did, bool show);

    Q_INVOKABLE bool getHideRepliesInThreadFromUnfollowed(const QString did) const;
    Q_INVOKABLE void setHideRepliesInThreadFromUnfollowed(const QString& did, bool hide);

    Q_INVOKABLE bool getAssembleThreads(const QString& did) const;
    Q_INVOKABLE void setAssembleThreads(const QString& did, bool assemble);

    Q_INVOKABLE bool getRewindToLastSeenPost(const QString& did) const;
    Q_INVOKABLE void setRewindToLastSeenPost(const QString& did, bool rewind);

    QStringList getRecentGifs(const QString& did) const; // Returns list of GIF id's
    void setRecentGifs(const QString& did, const QStringList& gifIds);

    QStringList getLastSearches(const QString& did) const;
    void setLastSearches(const QString& did, const QStringList& lastSearches);

    QStringList getLastProfileSearches(const QString& did) const;
    void setLastProfileSearches(const QString& did, const QStringList& lastDids);

    Q_INVOKABLE QEnums::ContentVisibility getSearchAdultOverrideVisibility(const QString& did);
    Q_INVOKABLE void setSearchAdultOverrideVisibility(const QString& did, QEnums::ContentVisibility visibility);

    bool getShowTrendingTopics() const;
    void setShowTrendingTopics(bool show);

    bool getShowSuggestedUsers() const;
    void setShowSuggestedUsers(bool show);

    QString getDefaultPostLanguage(const QString& did) const;
    void setDefaultPostLanguage(const QString& did, const QString& language);

    QStringList getUsedPostLanguages(const QString& did) const;
    void setUsedPostLanguages(const QString& did, const QStringList& languages);

    // Returns a sorted list
    Q_INVOKABLE QStringList getContentLanguages(const QString& did) const;
    Q_INVOKABLE void setContentLanguages(const QString& did, const QStringList& languages);

    // Returns a sorted list
    Q_INVOKABLE QStringList getExcludeDetectLanguages(const QString& did) const;
    Q_INVOKABLE void setExcludeDetectLanguages(const QString& did, const QStringList& languages);

    Q_INVOKABLE bool getShowUnknownContentLanguage(const QString& did) const;
    Q_INVOKABLE void setShowUnknownContentLanguage(const QString& did, bool show);

    bool getDefaultLanguageNoticeSeen() const;
    void setDefautlLanguageNoticeSeen(bool seen);

    Q_INVOKABLE bool getShowLanguageTags() const;
    Q_INVOKABLE void setShowLanguageTags(bool show);

    QDate getAnniversaryNoticeDate(const QString& did) const override;
    void setAnniversaryNoticeDate(const QString& did, QDate date) override;

    const QJsonDocument getFocusHashtags(const QString& did) const;
    void setFocusHashtags(const QString& did, const QJsonDocument& jsonHashtags);

    QStringList getLabels(const QString& did, const QString& labelerDid) const;
    void setLabels(const QString& did, const QString& labelerDid, const QStringList labels);
    void removeLabels(const QString& did, const QString& labelerDid);
    bool containsLabeler(const QString& did, const QString& labelerDid) const;

    SearchFeed::List getPinnedSearchFeeds(const QString& did) const;
    void setPinnedSearchFeeds(const QString& did, const SearchFeed::List& searchFeeds);

    void sync() { mSettings.sync(); }
    void syncLater() { QTimer::singleShot(0, [this]{ sync(); }); }

signals:
    void contentLanguageFilterChanged();
    void backgroundColorChanged();
    void accentColorChanged();
    void linkColorChanged();
    void threadStyleChanged();
    void threadColorChanged();
    void favoritesBarPositionChanged();
    void giantEmojisChanged();
    void songlinkEnabledChanged();
    void showFollowsActiveStatusChanged();
    void landscapeSideBarChanged();
    void gifAutoPlayChanged();
    void videoSoundChanged();
    void videoAutoPlayChanged();
    void videoAutoLoadChanged();
    void videoLoopPlayChanged();
    void videoQualityChanged();
    void videoStreamingEnabledChanged();
    void floatingNavigationButtonsChanged();
    void scriptRecognitionChanged();
    void showTrendingTopicsChanged();
    void showSuggestedUsersChanged();
    void blocksWithExpiryChanged();
    void mutesWithExpiryChanged();
    void feedHideRepliesChanged(QString did, QString feedUri);
    void feedHideFollowingChanged(QString did, QString feedUri);

private:
    QString key(const QString& did, const QString& subkey) const;
    QString uriKey(const QString& did, const QString& subkey, QString uri) const;
    QString displayKey(const QString& key) const;
    QString labelsKey(const QString& did, const QString& labelerDid) const;
    void cleanup();

    QStringList getFeedViewUris(const QString& did, const QString& feedKey) const;

    QSettings mSettings;
    PasswordEncryption mEncryption;
    std::optional<std::unordered_set<QString>> mSyncFeeds;
    std::unique_ptr<UriWithExpirySet> mBlocksWithExpiry;
    std::unique_ptr<UriWithExpirySet> mMutesWithExpiry;

    // Derived from display mode
    static QEnums::DisplayMode sActiveDisplayMode; // LIGHT or DARK
    static QString sDefaultBackgroundColor;
    static QString sCurrentLinkColor;

    // TODO: this cache only works for as long the calls to the settings are for the
    // same DID.
    // Cache
    mutable std::optional<bool> mHideRepliesInThreadFromUnfollowed;
    mutable std::optional<bool> mShowSelfReposts;
    mutable std::optional<bool> mShowFollowedReposts;
    mutable std::optional<bool> mShowUnknownContentLanguage;
    mutable std::optional<QStringList> mContentLanguages;
    mutable std::optional<bool> mShowQuotesWithBlockedPost;
    mutable std::optional<bool> mAssembleThreads;
};

}
