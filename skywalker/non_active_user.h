// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "notification_list_model.h"
#include "presence.h"
#include "post_view.h"
#include "profile.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

class SessionManager;

class NonActiveUser : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(BasicProfile profile READ getProfile CONSTANT FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(bool sessionExpired READ isSessionExpired CONSTANT FINAL)
    Q_PROPERTY(NotificationListModel* notificationListModel READ getNotificationListModel CONSTANT FINAL)
    Q_PROPERTY(PostView* postView READ getPostView NOTIFY postViewChanged FINAL)
    Q_PROPERTY(bool getPostInProgress READ isGetPostInProgress NOTIFY getPostInProgressChanged FINAL)
    Q_PROPERTY(bool actionInProgress READ isActionInProgress NOTIFY actionInProgressChanged FINAL)
    QML_ELEMENT

public:
    using Ptr = std::unique_ptr<NonActiveUser>;
    using List = QList<NonActiveUser*>;

    NonActiveUser(const BasicProfile& profile, bool sessionExpired, int notificationListModelId,
                  ATProto::Client* bsky, SessionManager* sessionManager, QObject* parent = nullptr);
    ~NonActiveUser();

    const BasicProfile& getProfile() const { return mProfile; }
    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);
    bool isSessionExpired() const { return mSessionExpired; }

    NotificationListModel* getNotificationListModel() const;
    Q_INVOKABLE void getNotifications(int limit = 25, bool updateSeen = false, const QString& cursor = {});
    Q_INVOKABLE void getNotificationsNextPage();

    PostView* getPostView() const;
    void setPostView(PostView::Ptr postView);
    void clearPostView();
    Q_INVOKABLE void getPost(const QString& uri);
    Q_INVOKABLE void like(const QString& viaUri = {}, const QString& viaCid = {});

    bool isGetPostInProgress() const { return mGetPostInProgress; }
    void setPostInProgress(bool inProgress);

    bool isActionInProgress() const { return mActionInProgress; }
    void setActionInProgress(bool inProgress);

signals:
    void unreadNotificationCountChanged();
    void postViewChanged();
    void getPostInProgressChanged();
    void actionInProgressChanged();

private:
    void doLike(const QString& viaUri, const QString& viaCid);
    void undoLike();

    ATProto::PostMaster* postMaster();

    BasicProfile mProfile;
    int mUnreadNotificationCount = 0;
    bool mSessionExpired = false;
    int mNotificationListModelId;
    ATProto::Client* mBsky = nullptr;
    SessionManager* mSessionManager;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    PostView::Ptr mPostView;
    bool mGetPostInProgress = false;
    bool mActionInProgress = false; // like, bookmark, repost, reply
};

}
