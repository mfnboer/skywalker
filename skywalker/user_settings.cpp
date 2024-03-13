// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_settings.h"
#include "definitions.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

static constexpr char const* KEY_ALIAS_PASSWORD = "SkywalkerPass";

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

void UserSettings::savePassword(const QString& did, const QString& password)
{
    const QByteArray encryptedPassword = mEncryption.encrypt(password, KEY_ALIAS_PASSWORD);
    mSettings.setValue(key(did, "password"), encryptedPassword);
}

QString UserSettings::getPassword(const QString& did) const
{
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
    mSettings.setValue(key(session.mDid, "handle"), session.mHandle);
    mSettings.setValue(key(session.mDid, "access"), session.mAccessJwt);
    mSettings.setValue(key(session.mDid, "refresh"), session.mRefreshJwt);
}

ATProto::ComATProtoServer::Session UserSettings::getSession(const QString& did) const
{
    ATProto::ComATProtoServer::Session session;
    session.mDid = did;
    session.mHandle = mSettings.value(key(did, "handle")).toString();
    session.mAccessJwt = mSettings.value(key(did, "access")).toString();
    session.mRefreshJwt = mSettings.value(key(did, "refresh")).toString();
    return session;
}

void UserSettings::clearCredentials(const QString& did)
{
    mSettings.setValue(key(did, "password"), {});
    mSettings.setValue(key(did, "access"), {});
    mSettings.setValue(key(did, "refresh"), {});
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

QStringList UserSettings::getBookmarks(const QString& did) const
{
    return mSettings.value(key(did, "bookmarks")).toStringList();
}

void UserSettings::removeBookmarks(const QString& did)
{
    qDebug() << "Remove locally stored muted bookmarks";
    mSettings.remove(key(did, "bookmarks"));
    mSettings.remove("bookmarksNoticeSeen");
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

void UserSettings::setGifAutoPlay(bool autoPlay)
{
    mSettings.setValue("gifAutoPlay", autoPlay);
}

bool UserSettings::getGifAutoPlay() const
{
    return mSettings.value("gifAutoPlay", true).toBool();
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

void UserSettings::setPullNotifications(const QString& did, bool enabled)
{
    mSettings.setValue(key(did, "pullNotifications"), enabled);
}

bool UserSettings::getPullNotifications(const QString& did) const
{
    return mSettings.value(key(did, "pullNotifications"), false).toBool();
}

void UserSettings::cleanup()
{
    // Version 1.5 erroneously saved user hashtags on app level
    mSettings.remove("userHashtags");
}

}
