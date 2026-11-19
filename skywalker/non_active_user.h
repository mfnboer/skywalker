// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "basic_profile_view.h"
#include "notification_list_model.h"
#include "presence.h"
#include "post_view.h"
#include "profile.h"
#include <atproto/lib/graph_master.h>
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
    Q_PROPERTY(BasicProfileView* profileForAction READ getProfileForAction NOTIFY profileForActionChanged FINAL)
    Q_PROPERTY(bool getActionObjectInProgress READ isGetActionObjectInProgress NOTIFY getActionObjectInProgressChanged FINAL)
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

    void init();

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

    BasicProfileView* getProfileForAction() const;
    void setProfileForAction(BasicProfileView::Ptr profile);
    void clearProfileForAction();

    void clearObjectsForAction();

    Q_INVOKABLE void getPost(const QString& uri);
    Q_INVOKABLE void getProfileForAction(const QString& did);

    Q_INVOKABLE void like(const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void repost(const QString& viaUri = {}, const QString& viaCid = {});
    Q_INVOKABLE void bookmark();
    Q_INVOKABLE void follow();

    bool isGetActionObjectInProgress() const { return mGetActionObjectInProgress; }
    void setActionObjectInProgress(bool inProgress);

    bool isActionInProgress() const { return mActionInProgress; }
    void setActionInProgress(bool inProgress);

signals:
    void unreadNotificationCountChanged();
    void sessionExpiredChanged();
    void postViewChanged();
    void profileForActionChanged();
    void getActionObjectInProgressChanged();
    void actionInProgressChanged();

private:
    void doLike(const QString& viaUri, const QString& viaCid);
    void undoLike();

    void doRepost(const QString& viaUri, const QString& viaCid);
    void continueDoRepost(const QString& uri, const QString& cid, const QString& viaUri, const QString& viaCid);
    void undoRepost();

    void addBookmark();
    void removeBookmark();

    void doFollow();
    void undoFollow();

    void removeNotificationListModel();

    ATProto::PostMaster* postMaster();
    ATProto::GraphMaster* graphMaster();

    BasicProfile mProfile;
    int mUnreadNotificationCount = 0;
    bool mSessionExpired = false;
    int mNotificationListModelId = -1;
    ATProto::Client::SharedPtr mBsky;
    std::unique_ptr<Skywalker> mSkywalker;
    SessionManager* mSessionManager = nullptr;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    std::unique_ptr<ATProto::GraphMaster> mGraphMaster;

    // Objects for action to operate on
    PostView::Ptr mPostView;
    BasicProfileView::Ptr mProfileForAction;

    bool mGetActionObjectInProgress = false;
    bool mActionInProgress = false; // like, bookmark, repost, reply
};

}
