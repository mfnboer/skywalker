// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_settings.h"

namespace Skywalker {

static constexpr char const* KEY_ALIAS_PASSWORD = "SkywalkerPass";
static constexpr char const* KEY_ALIAS_ACCESS_TOKEN = "SkywalkerAccess";
static constexpr char const* KEY_ALIAS_REFRESH_TOKEN = "SkywalkerRefresh";

UserSettings::UserSettings(QObject* parent) :
    QObject(parent)
{
    mEncryption.init(KEY_ALIAS_PASSWORD);
    mEncryption.init(KEY_ALIAS_ACCESS_TOKEN);
    mEncryption.init(KEY_ALIAS_REFRESH_TOKEN);
}

QString UserSettings::key(const QString& handle, const QString& subkey) const
{
    return QString("%1/%2").arg(handle, subkey);
}

QStringList UserSettings::getUsers() const
{
    return mSettings.value("users").toStringList();
}

void UserSettings::setActiveUser(const QString& handle)
{
    mSettings.setValue("activeUser", handle);
}

QString UserSettings::getActiveUser() const
{
    return mSettings.value("activeUser").toString();
}

void UserSettings::addUser(const QString& handle, const QString& host)
{
    auto users = getUsers();
    users.append(handle);
    users.sort();
    mSettings.setValue("users", users);
    mSettings.setValue(key(handle, "host"), host);
}

void UserSettings::removeUser(const QString& handle)
{
    auto users = getUsers();
    users.removeOne(handle);
    mSettings.setValue("users", users);
    clearCredentials(handle);

    const auto activeUser = getActiveUser();
    if (handle == activeUser)
        setActiveUser("");
}

QString UserSettings::getHost(const QString& handle) const
{
    return mSettings.value(key(handle, "host")).toString();
}

void UserSettings::savePassword(const QString& handle, const QString& password)
{
    const QByteArray encryptedPassword = mEncryption.encrypt(password, KEY_ALIAS_PASSWORD);
    mSettings.setValue(key(handle, "password"), encryptedPassword);
}

QString UserSettings::getPassword(const QString& handle) const
{
    const QByteArray encryptedPassword = mSettings.value(key(handle, "password")).toByteArray();

    if (encryptedPassword.isEmpty())
        return {};

    return mEncryption.decrypt(encryptedPassword, KEY_ALIAS_PASSWORD);
}

void UserSettings::saveDisplayName(const QString& handle, const QString& displayName)
{
    mSettings.setValue(key(handle, "displayName"), displayName);
}

QString UserSettings::getDisplayName(const QString& handle) const
{
    return mSettings.value(key(handle, "displayName")).toString();
}

void UserSettings::saveAvatar(const QString& handle, const QString& avatar)
{
    mSettings.setValue(key(handle, "avatar"), avatar);
}

QString UserSettings::getAvatar(const QString& handle) const
{
    return mSettings.value(key(handle, "avatar")).toString();
}

void UserSettings::saveSession(const QString& handle, const ATProto::ComATProtoServer::Session& session)
{
    const QByteArray encryptedAccessToken = mEncryption.encrypt(session.mAccessJwt, KEY_ALIAS_ACCESS_TOKEN);
    const QByteArray encryptedRefreshToken = mEncryption.encrypt(session.mRefreshJwt, KEY_ALIAS_REFRESH_TOKEN);

    mSettings.setValue(key(handle, "did"), session.mDid);
    mSettings.setValue(key(handle, "access"), encryptedAccessToken);
    mSettings.setValue(key(handle, "refresh"), encryptedRefreshToken);
}

ATProto::ComATProtoServer::Session UserSettings::getSession(const QString& handle) const
{
    ATProto::ComATProtoServer::Session session;
    session.mDid = mSettings.value(key(handle, "did")).toString();
    const QByteArray encryptedAccessToken = mSettings.value(key(handle, "access")).toByteArray();

    if (!encryptedAccessToken.isEmpty())
        session.mAccessJwt = mEncryption.decrypt(encryptedAccessToken, KEY_ALIAS_ACCESS_TOKEN);

    const QByteArray encryptedRefreshToken = mSettings.value(key(handle, "refresh")).toByteArray();

    if (!encryptedRefreshToken.isEmpty())
        session.mRefreshJwt = mEncryption.decrypt(encryptedRefreshToken, KEY_ALIAS_REFRESH_TOKEN);

    return session;
}

void UserSettings::clearCredentials(const QString& handle)
{
    mSettings.setValue(key(handle, "password"), "");
    mSettings.setValue(key(handle, "access"), "");
    mSettings.setValue(key(handle, "refresh"), "");
}

}
