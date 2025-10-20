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
class Skywalker;

class NonActiveUser : public QObject, public Presence
{
    Q_MOC_INCLUDE("skywalker.h")

    Q_OBJECT
    Q_PROPERTY(BasicProfile profile READ getProfile CONSTANT FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(bool sessionExpired READ isSessionExpired NOTIFY sessionExpiredChanged FINAL)
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker CONSTANT FINAL)
    Q_PROPERTY(NotificationListModel* notificationListModel READ getNotificationListModel CONSTANT FINAL)
    Q_PROPERTY(PostView* postView READ getPostView NOTIFY postViewChanged FINAL)
    Q_PROPERTY(bool getPostInProgress READ isGetPostInProgress NOTIFY getPostInProgressChanged FINAL)
    Q_PROPERTY(bool actionInProgress READ isActionInProgress NOTIFY actionInProgressChanged FINAL)
    QML_ELEMENT

public:
    using Ptr = std::unique_ptr<NonActiveUser>;
    using List = QList<NonActiveUser*>;

    NonActiveUser(QObject* parent = nullptr);
    NonActiveUser(const BasicProfile& profile, bool sessionExpired,
                  ATProto::Client::SharedPtr bsky, SessionManager* sessionManager,
                  QObject* parent = nullptr);
    ~NonActiveUser();

    const BasicProfile& getProfile() const { return mProfile; }

    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);

    bool isSessionExpired() const { return mSessionExpired; }
    void expireSession();

    Skywalker* getSkywalker() const;

    NotificationListModel* getNotificationListModel() const;
    Q_INVOKABLE void getNotifications(int limit = 25, bool updateSeen = false, const QString& cursor = {});
    Q_INVOKABLE void getNotificationsNextPage();

    PostView* getPostView() const;
    void setPostView(PostView::Ptr postView);
    void clearPostView();

    Q_INVOKABLE void getPost(const QString& uri);
    Q_INVOKABLE void like(const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void repost(const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void bookmark();

    bool isGetPostInProgress() const { return mGetPostInProgress; }
    void setPostInProgress(bool inProgress);

    bool isActionInProgress() const { return mActionInProgress; }
    void setActionInProgress(bool inProgress);

signals:
    void unreadNotificationCountChanged();
    void sessionExpiredChanged();
    void postViewChanged();
    void getPostInProgressChanged();
    void actionInProgressChanged();

private:
    void doLike(const QString& viaUri, const QString& viaCid);
    void undoLike();

    void doRepost(const QString& viaUri, const QString& viaCid);
    void continueDoRepost(const QString& uri, const QString& cid, const QString& viaUri, const QString& viaCid);
    void undoRepost();


    void addBookmark();
    void removeBookmark();

    void removeNotificationListModel();

    ATProto::PostMaster* postMaster();

    BasicProfile mProfile;
    int mUnreadNotificationCount = 0;
    bool mSessionExpired = false;
    int mNotificationListModelId = -1;
    ATProto::Client::SharedPtr mBsky;
    std::unique_ptr<Skywalker> mSkywalker;
    SessionManager* mSessionManager = nullptr;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    PostView::Ptr mPostView;
    bool mGetPostInProgress = false;
    bool mActionInProgress = false; // like, bookmark, repost, reply
};

}
