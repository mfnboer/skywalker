// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_settings.h"
#include "definitions.h"
#include <atproto/lib/at_uri.h>

// NOTE: do not store user defined types (Q_DECLARE_METATYPE) in settings.
// This will break OffLineMessageChecker. This runs in an Android background process.
// For some reason these types are not available in such a process.

namespace Skywalker {

static constexpr char const* KEY_ALIAS_PASSWORD = "SkywalkerPass";
static constexpr int MAX_ATTEMPTS_DRAFT_MIGRATION = 10;

QEnums::DisplayMode UserSettings::sActiveDisplayMode(QEnums::DISPLAY_MODE_LIGHT);
QString UserSettings::sDefaultBackgroundColor("white");
QString UserSettings::sLinkColor("blue");

UserSettings::UserSettings(QObject* parent) :
    QObject(parent)
{
    qDebug() << "Settings:" << mSettings.fileName();
    mEncryption.init(KEY_ALIAS_PASSWORD);
    cleanup();
}

UserSettings::UserSettings(const QString& fileName, QObject* parent) :
    QObject(parent),
    mSettings(fileName, QSettings::defaultFormat())
{
    qDebug() << "Settings:" << mSettings.fileName();
    mEncryption.init(KEY_ALIAS_PASSWORD);
    cleanup();
}

QString UserSettings::key(const QString& did, const QString& subkey) const
{
    return QString("%1/%2").arg(did, subkey);
}

QString UserSettings::displayKey(const QString& key) const
{
    const auto display = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "dark" : "light";
    return QString("%1/%2").arg(key, display);
}

QString UserSettings::labelsKey(const QString& did, const QString& labelerDid) const
{
    return QString("%1/labels/%2").arg(did, labelerDid);
}

void UserSettings::setActiveDisplayMode(QEnums::DisplayMode mode)
{
    if (mode != sActiveDisplayMode)
    {
        sActiveDisplayMode = mode;
        emit threadColorChanged();
        emit backgroundColorChanged();
    }
}

QList<BasicProfile> UserSettings::getUserList() const
{
    const auto didList = getUserDidList();
    QList<BasicProfile> userList;

    for (const auto& did : didList)
    {
        BasicProfile profile(
            did,
            getHandle(did),
            getDisplayName(did),
            getAvatar(did));

        userList.append(profile);
    }

    return userList;
}

QList<BasicProfile> UserSettings::getUserListWithAddAccount() const
{
    auto userList = getUserList();
    const BasicProfile addAccount("", "", tr("Add other acount"), "");
    userList.append(addAccount);
    return userList;
}

QStringList UserSettings::getUserDidList() const
{
    return mSettings.value("users").toStringList();
}

BasicProfile UserSettings::getUser(const QString& did) const
{
    BasicProfile profile(
        did,
        getHandle(did),
        getDisplayName(did),
        getAvatar(did));

    return profile;
}

void UserSettings::setActiveUserDid(const QString& did)
{
    mSettings.setValue("activeUser", did);
}

QString UserSettings::getActiveUserDid() const
{
    return mSettings.value("activeUser").toString();
}

void UserSettings::addUser(const QString& did, const QString& host)
{
    auto users = getUserDidList();

    if (users.contains(did))
    {
        qDebug() << "User already added:" << did << "host:" << host;
        return;
    }

    users.append(did);
    users.sort();
    mSettings.setValue("users", users);
    mSettings.setValue(key(did, "host"), host);
}

void UserSettings::removeUser(const QString& did)
{
    auto users = getUserDidList();
    users.removeOne(did);
    mSettings.setValue("users", users);
    clearCredentials(did);

    const auto activeUser = getActiveUserDid();
    if (did == activeUser)
        setActiveUserDid({});
}

QString UserSettings::getHost(const QString& did) const
{
    return mSettings.value(key(did, "host")).toString();
}

void UserSettings::setRememberPassword(const QString& did, bool enable)
{
    mSettings.setValue(key(did, "rememberPassword"), enable);

    if (!enable)
        mSettings.remove(key(did, "password"));

    sync();
}

bool UserSettings::getRememberPassword(const QString& did) const
{
    return mSettings.value(key(did, "rememberPassword"), false).toBool();
}

void UserSettings::savePassword(const QString& did, const QString& password)
{
    if (!getRememberPassword(did))
    {
        qWarning() << "Password saving is not enabled";
        return;
    }

    const QByteArray encryptedPassword = mEncryption.encrypt(password, KEY_ALIAS_PASSWORD);
    mSettings.setValue(key(did, "password"), encryptedPassword);
}

QString UserSettings::getPassword(const QString& did) const
{
    if (!getRememberPassword(did))
    {
        qWarning() << "Password saving is not enabled";
        return {};
    }

    const QByteArray encryptedPassword = mSettings.value(key(did, "password")).toByteArray();

    if (encryptedPassword.isEmpty())
        return {};

    return mEncryption.decrypt(encryptedPassword, KEY_ALIAS_PASSWORD);
}

QString UserSettings::getHandle(const QString& did) const
{
    return mSettings.value(key(did, "handle")).toString();
}

void UserSettings::saveDisplayName(const QString& did, const QString& displayName)
{
    mSettings.setValue(key(did, "displayName"), displayName);
}

QString UserSettings::getDisplayName(const QString& did) const
{
    return mSettings.value(key(did, "displayName")).toString();
}

void UserSettings::saveAvatar(const QString& did, const QString& avatar)
{
    mSettings.setValue(key(did, "avatar"), avatar);
}

QString UserSettings::getAvatar(const QString& did) const
{
    return mSettings.value(key(did, "avatar")).toString();
}

void UserSettings::saveSession(const ATProto::ComATProtoServer::Session& session)
{
    qDebug() << "Save session";
    mSettings.setValue(key(session.mDid, "handle"), session.mHandle);
    mSettings.setValue(key(session.mDid, "access"), session.mAccessJwt);
    mSettings.setValue(key(session.mDid, "refresh"), session.mRefreshJwt);
    mSettings.setValue(key(session.mDid, "2FA"), session.mEmailAuthFactor);
    mSettings.sync();
}

ATProto::ComATProtoServer::Session UserSettings::getSession(const QString& did) const
{
    ATProto::ComATProtoServer::Session session;
    session.mDid = did;
    session.mHandle = mSettings.value(key(did, "handle")).toString();
    session.mAccessJwt = mSettings.value(key(did, "access")).toString();
    session.mRefreshJwt = mSettings.value(key(did, "refresh")).toString();
    session.mEmailAuthFactor = mSettings.value(key(did, "2FA"), false).toBool();
    return session;
}

void UserSettings::clearTokens(const QString& did)
{
    qDebug() << "Clear tokens:" << did;
    mSettings.remove(key(did, "access"));
    mSettings.remove(key(did, "refresh"));
    mSettings.sync();
}

void UserSettings::clearCredentials(const QString& did)
{
    qDebug() << "Clear credentials:" << did;
    setRememberPassword(did, false);
    mSettings.remove(key(did, "password"));
    clearTokens(did);
}

void UserSettings::saveSyncTimestamp(const QString& did, QDateTime timestamp)
{
    mSettings.setValue(key(did, "syncTimestamp"), timestamp);
}

QDateTime UserSettings::getSyncTimestamp(const QString& did) const
{
    return mSettings.value(key(did, "syncTimestamp")).toDateTime();
}

void UserSettings::updateLastSignInTimestamp(const QString& did)
{
    mSettings.setValue(key(did, "lastSignInTimestamp"), QDateTime::currentDateTime());
}

QDateTime UserSettings::getLastSignInTimestamp(const QString& did) const
{
    return mSettings.value(key(did, "lastSignInTimestamp")).toDateTime();
}

void UserSettings::setBookmarks(const QString& did, const QStringList& bookmarks)
{
    mSettings.setValue(key(did, "bookmarks"), bookmarks);
}

QStringList UserSettings::getBookmarks(const QString& did) const
{
    return mSettings.value(key(did, "bookmarks")).toStringList();
}

QStringList UserSettings::getMutedWords(const QString& did) const
{
    return mSettings.value(key(did, "mutedWords")).toStringList();
}

void UserSettings::removeMutedWords(const QString& did)
{
    qDebug() << "Remove locally stored muted words";
    mSettings.remove(key(did, "mutedWords"));
    mSettings.remove("mutedWordsNoticeSeen");
}

void UserSettings::setDisplayMode(QEnums::DisplayMode displayMode)
{
    mSettings.setValue("displayMode", (int)displayMode);
}

QEnums::DisplayMode UserSettings::getDisplayMode() const
{
    const int mode = mSettings.value("displayMode", (int)QEnums::DISPLAY_MODE_SYSTEM).toInt();

    if (mode < QEnums::DISPLAY_MODE_SYSTEM || mode > QEnums::DISPLAY_MODE_DARK)
        return QEnums::DISPLAY_MODE_SYSTEM;

    return QEnums::DisplayMode(mode);
}

void UserSettings::resetBackgroundColor()
{
    mSettings.remove(displayKey("backgroundColor"));
    emit backgroundColorChanged();
}

void UserSettings::setBackgroundColor(const QString& color)
{
    if (getBackgroundColor() != color)
    {
        mSettings.setValue(displayKey("backgroundColor"), color);
        emit backgroundColorChanged();
    }
}

QString UserSettings::getBackgroundColor() const
{
    return mSettings.value(displayKey("backgroundColor"), getDefaultBackgroundColor()).toString();
}

void UserSettings::resetAccentColor()
{
    mSettings.remove(displayKey("accentColor"));
    emit accentColorChanged();
}

void UserSettings::setAccentColor(const QString& color)
{
    if (getAccentColor() != color)
    {
        mSettings.setValue(displayKey("accentColor"), color);
        emit accentColorChanged();
    }
}

QString UserSettings::getAccentColor() const
{
    const QString defaultColor = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "#58a6ff" : "blue";
    return mSettings.value(displayKey("accentColor"), defaultColor).toString();
}

void UserSettings::setThreadStyle(QEnums::ThreadStyle threadStyle)
{
    const auto oldStyle = getThreadStyle();
    mSettings.setValue("threadStyle", (int)threadStyle);

    if (oldStyle != threadStyle)
        emit threadStyleChanged();
}

QEnums::ThreadStyle UserSettings::getThreadStyle() const
{
    const int style = mSettings.value("threadStyle", (int)QEnums::THREAD_STYLE_BAR).toInt();

    if (style < QEnums::THREAD_STYLE_BAR || style > QEnums::THREAD_STYLE_LINE)
        return QEnums::THREAD_STYLE_BAR;

    return QEnums::ThreadStyle(style);
}

void UserSettings::resetThreadColor()
{
    mSettings.remove(displayKey("threadColor"));
    emit threadColorChanged();
}

void UserSettings::setThreadColor(const QString& color)
{
    if (getThreadColor() != color)
    {
        mSettings.setValue(displayKey("threadColor"), color);
        emit threadColorChanged();
    }
}

QString UserSettings::getThreadColor() const
{
    const QString defaultColor = getActiveDisplayMode() == QEnums::DISPLAY_MODE_DARK ? "#000080" : "#8080ff";
    return mSettings.value(displayKey("threadColor"), defaultColor).toString();
}

void UserSettings::setPostButtonRelativeX(double x)
{
    mSettings.setValue("postButtonRelativeX", x);
}

double UserSettings::getPostButtonRelativeX() const
{
    return mSettings.value("postButtonRelativeX", 1.0).toDouble();
}

void UserSettings::setGifAutoPlay(bool autoPlay)
{
    mSettings.setValue("gifAutoPlay", autoPlay);
}

bool UserSettings::getGifAutoPlay() const
{
    return mSettings.value("gifAutoPlay", true).toBool();
}

void UserSettings::setVideoAutoPlay(bool autoPlay)
{
    if (autoPlay == getVideoAutoPlay())
        return;

    mSettings.setValue("videoAutoPlay", autoPlay);
    emit videoAutoPlayChanged();
}

bool UserSettings::getVideoAutoPlay() const
{
    return mSettings.value("videoAutoPlay", false).toBool();
}

void UserSettings::setVideoAutoLoad(bool autoLoad)
{
    if (autoLoad == getVideoAutoLoad())
        return;

    mSettings.setValue("videoAutoLoad", autoLoad);
    emit videoAutoLoadChanged();
}

bool UserSettings::getVideoAutoLoad() const
{
    return mSettings.value("videoAutoLoad", false).toBool();
}

void UserSettings::setVideoSound(bool on)
{
    if (on == getVideoSound())
        return;

    mSettings.setValue("videoSound", on);
    emit videoSoundChanged();
}

bool UserSettings::getVideoSound() const
{
    return mSettings.value("videoSound", true).toBool();
}

void UserSettings::setGiantEmojis(bool giantEmojis)
{
    if (giantEmojis == getGiantEmojis())
        return;

    mSettings.setValue("giantEmojis", giantEmojis);
    emit giantEmojisChanged();
}

bool UserSettings::getGiantEmojis() const
{
    return mSettings.value("giantEmojis", true).toBool();
}

void UserSettings::setRequireAltText(const QString& did, bool require)
{
    mSettings.setValue(key(did, "requireAltText"), require);
}

bool UserSettings::getRequireAltText(const QString& did) const
{
    return mSettings.value(key(did, "requireAltText"), false).toBool();
}

QString UserSettings::getMutedRepostsListUri(const QString& did) const
{
    const ATProto::ATUri uri(did, ATProto::ATUri::COLLECTION_GRAPH_LIST, RKEY_MUTED_REPOSTS);
    return uri.toString();
}

void UserSettings::setThreadAutoNumber(bool autoNumber)
{
    mSettings.setValue("threadAutoNumber", autoNumber);
}

bool UserSettings::getThreadAutoNumber() const
{
    return mSettings.value("threadAutoNumber", true).toBool();
}

void UserSettings::setThreadPrefix(QString prefix)
{
    mSettings.setValue("threadPrefix", prefix);
}

QString UserSettings::getThreadPrefix() const
{
    return mSettings.value("threadPrefix", "🧵").toString();
}

void UserSettings::setThreadAutoSplit(bool autoSplit)
{
    mSettings.setValue("threadAutoSplit", autoSplit);
}

bool UserSettings::getThreadAutoSplit() const
{
    return mSettings.value("threadAutoSplit", false).toBool();
}

void UserSettings::setUserHashtags(const QString& did, const QStringList& hashtags)
{
    qDebug() << "Save user hashtags:" << did;
    mSettings.setValue(key(did, "userHashtags"), hashtags);
}

QStringList UserSettings::getUserHashtags(const QString& did) const
{
    return mSettings.value(key(did, "userHashtags")).toStringList();
}

void UserSettings::setSeenHashtags(const QStringList& hashtags)
{
    qDebug() << "Save seen hashtags";
    mSettings.setValue("seenHashtags", hashtags);
}

QStringList UserSettings::getSeenHashtags() const
{
    return mSettings.value("seenHashtags").toStringList();
}

void UserSettings::setOfflineUnread(const QString& did, int unread)
{
    mSettings.setValue(key(did, "offlineUnread"), unread);
}

int UserSettings::getOfflineUnread(const QString& did) const
{
    return mSettings.value(key(did, "offlineUnread"), 0).toInt();
}

void UserSettings::setOfflineMessageCheckTimestamp(QDateTime timestamp)
{
    mSettings.setValue("offlineMessageCheckTimestamp", timestamp);
}

QDateTime UserSettings::getOfflineMessageCheckTimestamp() const
{
    return mSettings.value("offlineMessageCheckTimestamp").toDateTime();
}

void UserSettings::setOffLineChatCheckRev(const QString& did, const QString& rev)
{
    mSettings.setValue(key(did, "offlineChatCheckRev"), rev);
}

QString UserSettings::getOffLineChatCheckRev(const QString& did) const
{
    return mSettings.value(key(did, "offlineChatCheckRev")).toString();
}

void UserSettings::setCheckOfflineChat(const QString& did, bool check)
{
    mSettings.setValue(key(did, "checkOfflineChat"), check);
}

bool UserSettings::mustCheckOfflineChat(const QString& did) const
{
    return mSettings.value(key(did, "checkOfflineChat"), false).toBool();
}

void UserSettings::resetNextNotificationId()
{
    mSettings.setValue("nextNotificationId", 1);
}

int UserSettings::getNextNotificationId()
{
    int id = mSettings.value("nextNotificationId", 1).toInt();
    int nextId = id + 1;
    mSettings.setValue("nextNotificationId", nextId);
    return id;
}

void UserSettings::setNotificationsWifiOnly(bool enable)
{
    mSettings.setValue("notificationsWifiOnly", enable);
}

bool UserSettings::getNotificationsWifiOnly() const
{
    return mSettings.value("notificationsWifiOnly", false).toBool();
}

bool UserSettings::getShowQuotesWithBlockedPost(const QString& did) const
{
    return mSettings.value(key(did, "showQuotesWithBlockedPost"), true).toBool();
}

void UserSettings::setShowQuotesWithBlockedPost(const QString& did, bool show)
{
    mSettings.setValue(key(did, "showQuotesWithBlockedPost"), show);
}

bool UserSettings::getRewindToLastSeenPost(const QString& did) const
{
    return mSettings.value(key(did, "rewindToLastSeenPost"), true).toBool();
}

void UserSettings::setRewindToLastSeenPost(const QString& did, bool rewind)
{
    mSettings.setValue(key(did, "rewindToLastSeenPost"), rewind);
}

QStringList UserSettings::getRecentGifs(const QString& did) const
{
    const QStringList list = mSettings.value(key(did, "recentGifs")).toStringList();
    QStringList gifIdList;

    for (const auto& s : list)
    {
        if (s.isEmpty())
            continue;

        const auto json = QJsonDocument::fromJson(s.toUtf8());
        const ATProto::XJsonObject& xjson(json.object());
        const auto id = xjson.getOptionalString("id");

        if (id)
            gifIdList.push_back(*id);
    }

    return gifIdList;
}

void UserSettings::setRecentGifs(const QString& did, const QStringList& gifIds)
{
    QStringList list;

    // The ID's are stored as JSON objects in older versions we stored the all
    // GIF attributes in a JSON object.
    for (const auto& id : gifIds) {
        QJsonObject json;
        json.insert("id", id);
        const QJsonDocument jsonDoc(json);
        list.push_back(jsonDoc.toJson(QJsonDocument::Compact));
    }

    mSettings.setValue(key(did, "recentGifs"), list);
}

QStringList UserSettings::getLastSearches(const QString& did) const
{
    return mSettings.value(key(did, "lastSearches")).toStringList();
}

void UserSettings::setLastSearches(const QString& did, const QStringList& lastSearches)
{
    mSettings.setValue(key(did, "lastSearches"), lastSearches);
}

void UserSettings::addDraftRepoToFileMigration(const QString& did)
{
    const int attempts = mSettings.value(key(did, "draftRepoToFileMigration"), 0).toInt();
    mSettings.setValue(key(did, "draftRepoToFileMigration"), attempts + 1);
}

QString UserSettings::getDefaultPostLanguage(const QString& did) const
{
    return mSettings.value(key(did, "defaultPostLanguage")).toString();
}

void UserSettings::setDefaultPostLanguage(const QString& did, const QString& language)
{
    mSettings.setValue(key(did, "defaultPostLanguage"), language);
}

QStringList UserSettings::getUsedPostLanguages(const QString& did) const
{
    return mSettings.value(key(did, "usedPostLanguages")).toStringList();
}

void UserSettings::setUsedPostLanguages(const QString& did, const QStringList& languages)
{
    mSettings.setValue(key(did, "usedPostLanguages"), languages);
}

QStringList UserSettings::getContentLanguages(const QString& did) const
{
    return mSettings.value(key(did, "contentLanguages")).toStringList();
}

void UserSettings::setContentLanguages(const QString& did, const QStringList& languages)
{
    const auto oldLangs = getContentLanguages(did);
    QStringList sortedLangs = languages;
    std::sort(sortedLangs.begin(), sortedLangs.end());

    if (oldLangs != sortedLangs)
    {
        mSettings.setValue(key(did, "contentLanguages"), sortedLangs);
        emit contentLanguageFilterChanged();
    }
}

bool UserSettings::getShowUnknownContentLanguage(const QString& did) const
{
    return mSettings.value(key(did, "showUnknownContentLanguage"), true).toBool();
}

void UserSettings::setShowUnknownContentLanguage(const QString& did, bool show)
{
    const bool oldShow = getShowUnknownContentLanguage(did);

    if (oldShow != show)
    {
        mSettings.setValue(key(did, "showUnknownContentLanguage"), show);
        emit contentLanguageFilterChanged();
    }
}

bool UserSettings::getDefaultLanguageNoticeSeen() const
{
    return mSettings.value("defaultLanguageNoticeSeen", false).toBool();
}

void UserSettings::setDefautlLanguageNoticeSeen(bool seen)
{
    mSettings.setValue("defaultLanguageNoticeSeen", seen);
}

bool UserSettings::getShowLanguageTags() const
{
    return mSettings.value("showLanguageTags", false).toBool();
}

void UserSettings::setShowLanguageTags(bool show)
{
    mSettings.setValue("showLanguageTags", show);
}

QDate UserSettings::getAnniversaryNoticeDate(const QString& did) const
{
    return mSettings.value(key(did, "anniversaryNoticeDate")).toDate();
}

void UserSettings::setAnniversaryNoticeDate(const QString& did, QDate date)
{
    mSettings.setValue(key(did, "anniversaryNoticeDate"), date);
}

const QJsonDocument UserSettings::getFocusHashtags(const QString& did) const
{
    return mSettings.value(key(did, "focusHashtags")).toJsonDocument();
}

void UserSettings::setFocusHashtags(const QString& did, const QJsonDocument& jsonHashtags)
{
    mSettings.setValue(key(did, "focusHashtags"), jsonHashtags);
}

QStringList UserSettings::getLabels(const QString& did, const QString& labelerDid) const
{
    return mSettings.value(labelsKey(did, labelerDid)).toStringList();
}

void UserSettings::setLabels(const QString& did, const QString& labelerDid, const QStringList labels)
{
    mSettings.setValue(labelsKey(did, labelerDid), labels);
}

void UserSettings::removeLabels(const QString& did, const QString& labelerDid)
{
    mSettings.remove(labelsKey(did, labelerDid));
}

bool UserSettings::containsLabeler(const QString& did, const QString& labelerDid) const
{
    return mSettings.contains(labelsKey(did, labelerDid));
}

void UserSettings::setDraftRepoToFileMigrationDone(const QString& did)
{
    mSettings.setValue(key(did, "draftRepoToFileMigration"), MAX_ATTEMPTS_DRAFT_MIGRATION);
}

bool UserSettings::isDraftRepoToFileMigrationDone(const QString& did) const
{
    const int attempts = mSettings.value(key(did, "draftRepoToFileMigration"), 0).toInt();
    return attempts >= MAX_ATTEMPTS_DRAFT_MIGRATION;
}

void UserSettings::cleanup()
{
    // Version 1.5 erroneously saved user hashtags on app level
    mSettings.remove("userHashtags");
}

}
