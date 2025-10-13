// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "non_active_user.h"
#include "user_settings.h"
#include <atproto/lib/client.h>
#include <QObject>
#include <unordered_map>

namespace Skywalker {

class Skywalker;

class SessionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int activeUserUnreadNotificationCount READ getActiveUserUnreadNotificationCount NOTIFY activeUserUnreadNotificationCountChanged FINAL)
    Q_PROPERTY(NonActiveUser::List nonActiveUsers READ getNonActiveUsers NOTIFY nonActiveUsersChanged FINAL)

public:
    using SuccessCb = std::function<void()>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    explicit SessionManager(Skywalker* skywalker, QObject* parent = nullptr);

    void clear();

    void insertSession(const QString& did, ATProto::Client* client);
    void resumeAndRefreshNonActiveUsers();
    bool resumeAndRefreshSession(const QString& did);
    void resumeAndRefreshSession(ATProto::Client* client, const ATProto::ComATProtoServer::Session& session,
                                 int refreshDelayCount = 0, const SuccessCb& successCb = nullptr,
                                 const ErrorCb& errorCb = nullptr);

    void startRefreshTimers();
    void stopRefreshTimers();

    void saveTokens();
    void updateTokens();

    void setUnreadNotificationCount(const QString& did, int unread);
    int getUnreadNotificationCount(const QString& did) const;
    int getTotalUnreadNotificationCount() const;
    int getActiveUserUnreadNotificationCount() const;

    const NonActiveUser::List& getNonActiveUsers() const { return mNonActiveUsers; }

    ATProto::Client* getActiveUserBskyClient() const;
    void showStatusMessage(const QString& msg, QEnums::StatusLevel level);

    void refreshAllData();
    void makeLocalModelChange(const std::function<void(LocalProfileChanges*)>& update);
    void makeLocalModelChange(const std::function<void(LocalPostModelChanges*)>& update);

signals:
    void activeSessionExpired(const QString& msg);
    void totalUnreadNotificationCountChanged(int count);
    void activeUserUnreadNotificationCountChanged();
    void nonActiveUsersChanged();

private:
    struct Session
    {
        using Ptr = std::unique_ptr<Session>;

        ~Session()
        {
            if (mNonActiveUser)
                delete mNonActiveUser;
        }

        ATProto::Client* mBsky = nullptr;
        ATProto::Client::Ptr mRawBsky;
        int mUnreadNotificationCount = 0;
        QTimer mRefreshNotificationInitialDelayTimer;
        QTimer mRefreshNotificationTimer;
        NonActiveUser* mNonActiveUser = nullptr;
    };

    Session::Ptr createSession(const QString& did, ATProto::Client::Ptr rawBsky, ATProto::Client* bsky);
    void insertSession(const QString& did, Session::Ptr session);
    void deleteSession(const QString& did);
    void addNonActiveUser(NonActiveUser* nonActiveUser);
    void addExpiredUser(const QString& did);
    Session* getSession(const QString& did) const;
    std::optional<ATProto::ComATProtoServer::Session> getSavedSession(const QString& did) const;
    void startRefreshTimers(const QString& did, int initialDelayCount);
    void stopRefreshTimers(const QString& did);
    void refreshNotificationCount(const QString& did);

    std::unordered_map<QString, Session::Ptr> mDidSessionMap;
    Skywalker* mSkywalker;
    UserSettings& mUserSettings;
    NonActiveUser::List mNonActiveUsers;
    std::vector<NonActiveUser::Ptr> mExpiredUsers;
};

}
