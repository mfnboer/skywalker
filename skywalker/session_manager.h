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
public:
    explicit SessionManager(UserSettings& userSettings, QObject* parent = nullptr);

    void resumeAndRefreshNonActiveUsers();
    void clear();

    bool resumeAndRefreshSession(const QString& did);
    void startRefreshTimers();
    void stopRefreshTimers();
    void saveTokens();
    void updateTokens();

private:
    struct Session
    {
        using Ptr = std::unique_ptr<Session>;

        ATProto::Client::Ptr mBsky;
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
    void setUnreadNotificationCount(const QString& did, int unread);

    std::unordered_map<QString, Session::Ptr> mDidSessionMap;
    UserSettings& mUserSettings;
};

}
