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

class UserSettings : public QObject
{
    Q_OBJECT
public:
    static QString getLinkColor() { return sLinkColor; }
    Q_INVOKABLE static void setLinkColor(const QString& color) { sLinkColor = color; }

    explicit UserSettings(QObject* parent = nullptr);

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

    void savePassword(const QString& did, const QString& password);
    Q_INVOKABLE QString getPassword(const QString& did) const;

    QString getHandle(const QString& did) const;

    void saveDisplayName(const QString& did, const QString& displayName);
    QString getDisplayName(const QString& did) const;

    void saveAvatar(const QString& did, const QString& avatar);
    QString getAvatar(const QString& did) const;

    void saveSession(const ATProto::ComATProtoServer::Session& session);
    ATProto::ComATProtoServer::Session getSession(const QString& did) const;

    void clearCredentials(const QString& did);

    void saveSyncTimestamp(const QString& did, QDateTime timestamp);
    QDateTime getSyncTimestamp(const QString& did) const;

    Q_INVOKABLE void updateLastSignInTimestamp(const QString& did);
    Q_INVOKABLE QDateTime getLastSignInTimestamp(const QString& did) const;

    void saveBookmarks(const QString& did, const QStringList& postUris);
    QStringList getBookmarks(const QString& did) const;

    void setBookmarksNoticeSeen(bool seen);
    bool getBookmarksNoticeSeen() const;

    void saveMutedWords(const QString& did, const QStringList& mutedWords);
    QStringList getMutedWords(const QString& did) const;

    void setMutedWordsNoticeSeen(bool seen);
    bool getMutedWordsNoticeSeen() const;

    void setDisplayMode(QEnums::DisplayMode displayMode);
    Q_INVOKABLE QEnums::DisplayMode getDisplayMode() const;

    void setGifAutoPlay(bool autoPlay);
    Q_INVOKABLE bool getGifAutoPlay() const;

private:
    QString key(const QString& did, const QString& subkey) const;

    QSettings mSettings;
    PasswordEncryption mEncryption;

    // Derived from display mode
    static QString sLinkColor;
};

}
