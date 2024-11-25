// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "password_encryption.h"
#include "profile.h"
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
    Q_PROPERTY(bool giantEmojis READ getGiantEmojis WRITE setGiantEmojis NOTIFY giantEmojisChanged FINAL)
    Q_PROPERTY(bool videoSound READ getVideoSound WRITE setVideoSound NOTIFY videoSoundChanged FINAL)
    Q_PROPERTY(bool videoAutoPlay READ getVideoAutoPlay WRITE setVideoAutoPlay NOTIFY videoAutoPlayChanged FINAL)
    Q_PROPERTY(bool videoAutoLoad READ getVideoAutoLoad WRITE setVideoAutoLoad NOTIFY videoAutoLoadChanged FINAL)
    Q_PROPERTY(bool floatingNavigationButtons READ getFloatingNavigationButtons WRITE setFloatingNavigationButtons NOTIFY floatingNavigationButtonsChanged FINAL)

public:
    static QEnums::DisplayMode getActiveDisplayMode() { return sActiveDisplayMode; }
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

    Q_INVOKABLE void updateLastSignInTimestamp(const QString& did);
    Q_INVOKABLE QDateTime getLastSignInTimestamp(const QString& did) const;

    // For the home feed, the URI is "home"
    Q_INVOKABLE void setLastViewedFeed(const QString& did, const QString& uri);
    Q_INVOKABLE QString getLastViewedFeed(const QString& did) const;

    void setBookmarks(const QString& did, const QStringList& bookmarks);
    QStringList getBookmarks(const QString& did) const;

    // Legacy
    QStringList getMutedWords(const QString& did) const;
    void removeMutedWords(const QString& did);

    void setDisplayMode(QEnums::DisplayMode displayMode);
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

    Q_INVOKABLE void setPostButtonRelativeX(double x);
    Q_INVOKABLE double getPostButtonRelativeX() const;

    void setGifAutoPlay(bool autoPlay);
    Q_INVOKABLE bool getGifAutoPlay() const;

    void setVideoAutoPlay(bool autoPlay);
    bool getVideoAutoPlay() const;

    void setVideoAutoLoad(bool autoLoad);
    bool getVideoAutoLoad() const;

    void setVideoSound(bool on);
    bool getVideoSound() const;

    void setGiantEmojis(bool giantEmojis);
    bool getGiantEmojis() const;

    void setFloatingNavigationButtons(bool floating);
    bool getFloatingNavigationButtons() const;

    Q_INVOKABLE void setRequireAltText(const QString& did, bool require);
    Q_INVOKABLE bool getRequireAltText(const QString& did) const;

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

    void setNotificationsWifiOnly(bool enable);
    bool getNotificationsWifiOnly() const;

    bool getShowQuotesWithBlockedPost(const QString& did) const;
    void setShowQuotesWithBlockedPost(const QString& did, bool show);

    Q_INVOKABLE bool getHideRepliesInThreadFromUnfollowed(const QString did) const;
    Q_INVOKABLE void setHideRepliesInThreadFromUnfollowed(const QString& did, bool hide);

    bool getRewindToLastSeenPost(const QString& did) const;
    void setRewindToLastSeenPost(const QString& did, bool rewind);

    QStringList getRecentGifs(const QString& did) const; // Returns list of GIF id's
    void setRecentGifs(const QString& did, const QStringList& gifIds);

    QStringList getLastSearches(const QString& did) const;
    void setLastSearches(const QString& did, const QStringList& lastSearches);

    QString getDefaultPostLanguage(const QString& did) const;
    void setDefaultPostLanguage(const QString& did, const QString& language);

    QStringList getUsedPostLanguages(const QString& did) const;
    void setUsedPostLanguages(const QString& did, const QStringList& languages);

    // Returns a sorted list
    QStringList getContentLanguages(const QString& did) const;
    void setContentLanguages(const QString& did, const QStringList& languages);

    bool getShowUnknownContentLanguage(const QString& did) const;
    void setShowUnknownContentLanguage(const QString& did, bool show);

    bool getDefaultLanguageNoticeSeen() const;
    void setDefautlLanguageNoticeSeen(bool seen);

    Q_INVOKABLE bool getShowLanguageTags() const;
    void setShowLanguageTags(bool show);

    QDate getAnniversaryNoticeDate(const QString& did) const override;
    void setAnniversaryNoticeDate(const QString& did, QDate date) override;

    const QJsonDocument getFocusHashtags(const QString& did) const;
    void setFocusHashtags(const QString& did, const QJsonDocument& jsonHashtags);

    QStringList getLabels(const QString& did, const QString& labelerDid) const;
    void setLabels(const QString& did, const QString& labelerDid, const QStringList labels);
    void removeLabels(const QString& did, const QString& labelerDid);
    bool containsLabeler(const QString& did, const QString& labelerDid) const;

    void addDraftRepoToFileMigration(const QString& did);
    void setDraftRepoToFileMigrationDone(const QString& did);
    bool isDraftRepoToFileMigrationDone(const QString& did) const;

    void sync() { mSettings.sync(); }

signals:
    void contentLanguageFilterChanged();
    void backgroundColorChanged();
    void accentColorChanged();
    void linkColorChanged();
    void threadStyleChanged();
    void threadColorChanged();
    void giantEmojisChanged();
    void videoSoundChanged();
    void videoAutoPlayChanged();
    void videoAutoLoadChanged();
    void floatingNavigationButtonsChanged();

private:
    QString key(const QString& did, const QString& subkey) const;
    QString displayKey(const QString& key) const;
    QString labelsKey(const QString& did, const QString& labelerDid) const;
    void cleanup();

    QSettings mSettings;
    PasswordEncryption mEncryption;

    // Derived from display mode
    static QEnums::DisplayMode sActiveDisplayMode; // LIGHT or DARK
    static QString sDefaultBackgroundColor;
    static QString sCurrentLinkColor;
};

}
