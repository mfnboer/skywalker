// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "password_encryption.h"
#include <atproto/lib/client.h>
#include <QObject>
#include <QSettings>

namespace Skywalker {

class UserSettings : public QObject
{
    Q_OBJECT
public:
    explicit UserSettings(QObject* parent = nullptr);

    Q_INVOKABLE QStringList getUsers() const;

    void setActiveUser(const QString& handle);
    QString getActiveUser() const;
    void addUser(const QString& handle, const QString& host);
    void removeUser(const QString& handle);
    QString getHost(const QString& handle) const;
    void savePassword(const QString& handle, const QString& password);
    QString getPassword(const QString& handle) const;
    void saveDisplayName(const QString& handle, const QString& displayName);
    QString getDisplayName(const QString& handle) const;
    void saveAvatar(const QString& handle, const QString& avatar);
    QString getAvatar(const QString& handle) const;
    void saveSession(const QString& handle, const ATProto::ComATProtoServer::Session& session);
    ATProto::ComATProtoServer::Session getSession(const QString& handle) const;
    void clearCredentials(const QString& handle);

private:
    QString key(const QString& handle, const QString& subkey) const;

    QSettings mSettings;
    PasswordEncryption mEncryption;
};

}
