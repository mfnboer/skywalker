// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "notification_list_model.h"
#include "profile.h"

namespace Skywalker {

class SessionManager;

class NonActiveUser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BasicProfile profile READ getProfile CONSTANT FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(bool sessionExpired READ isSessionExpired CONSTANT FINAL)
    Q_PROPERTY(NotificationListModel* notificationListModel READ getNotificationListModel CONSTANT FINAL)

public:
    using Ptr = std::unique_ptr<NonActiveUser>;
    using List = QList<NonActiveUser*>;

    NonActiveUser(const BasicProfile& profile, bool sessionExpired, NotificationListModel* notificationListModel,
                  ATProto::Client* bsky, SessionManager* sessionManager, QObject* parent = nullptr);

    const BasicProfile& getProfile() const { return mProfile; }
    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);
    bool isSessionExpired() const { return mSessionExpired; }

    NotificationListModel* getNotificationListModel() const { return mNotificationListModel; }
    Q_INVOKABLE void getNotifications(int limit = 25, bool updateSeen = false, const QString& cursor = {});
    Q_INVOKABLE void getNotificationsNextPage();

signals:
    void unreadNotificationCountChanged();

private:
    BasicProfile mProfile;
    int mUnreadNotificationCount = 0;
    bool mSessionExpired = false;
    NotificationListModel* mNotificationListModel;
    ATProto::Client* mBsky = nullptr;
    SessionManager* mSessionManager;
};

}
