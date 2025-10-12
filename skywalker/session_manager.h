// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "user_settings.h"
#include <atproto/lib/client.h>
#include <QObject>
#include <unordered_map>

namespace Skywalker {

class SessionManager : public QObject
{
    Q_OBJECT

public:
    using SuccessCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    explicit SessionManager(UserSettings& userSettings, QObject* parent = nullptr);

    void resumeAndRefreshNonActiveUsers();
    void clear();

    void insertSession(const QString& did, ATProto::Client* client);
    bool resumeAndRefreshSession(const QString& did);
    void resumeAndRefreshSession(ATProto::Client* client, const ATProto::ComATProtoServer::Session& session,
                                 int refreshDelayCount = 0, const SuccessCb& successCb = nullptr,
                                 const ErrorCb& errorCb = nullptr);
    void startRefreshTimers();
    void stopRefreshTimers();
    void saveTokens();
    void updateTokens();
    void setUnreadNotificationCount(const QString& did, int unread);

signals:
    void activeSessionExpired(const QString& msg);
    void unreadNotificationCountChanged(const QString& did, int count);

private:
    struct Session
    {
        using Ptr = std::unique_ptr<Session>;

        ATProto::Client* mBsky = nullptr;
        ATProto::Client::Ptr mRawBsky;
        int mUnreadNotificationCount = 0;
        QTimer mRefreshNotificationInitialDelayTimer;
        QTimer mRefreshNotificationTimer;
    };

    Session::Ptr createSession(const QString& did);
    Session* getSession(const QString& did);
    std::optional<ATProto::ComATProtoServer::Session> getSavedSession(const QString& did) const;
    void startRefreshTimers(const QString& did, int initialDelayCount);
    void stopRefreshTimers(const QString& did);
    void refreshNotificationCount(const QString& did);

    std::unordered_map<QString, Session::Ptr> mDidSessionMap;
    UserSettings& mUserSettings;
};

}
