// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "non_active_user.h"
#include "session_manager.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 50;

NonActiveUser::NonActiveUser(const BasicProfile& profile, bool sessionExpired, int notificationListModelId,
                             ATProto::Client* bsky, SessionManager* sessionManager, QObject* parent) :
    QObject(parent),
    mProfile(profile),
    mSessionExpired(sessionExpired),
    mNotificationListModelId(notificationListModelId),
    mBsky(bsky),
    mSessionManager(sessionManager)
{
    Q_ASSERT(mSessionManager);
}

NonActiveUser::~NonActiveUser()
{
    if (mNotificationListModelId >= 0)
        mSessionManager->removeNotificationListModel(mNotificationListModelId);
}

NotificationListModel* NonActiveUser::getNotificationListModel() const
{
    return mSessionManager->getNotificationListModel(mNotificationListModelId);
}

void NonActiveUser::setUnreadNotificationCount(int unread)
{
    if (unread == mUnreadNotificationCount)
        return;

    mUnreadNotificationCount = unread;
    emit unreadNotificationCountChanged();
}

void NonActiveUser::getNotifications(int limit, bool updateSeen, const QString& cursor)
{
    qDebug() << "Get notifications:" << cursor;

    if (!mBsky)
    {
        qWarning() << "No atproto client:" << mProfile.getHandle();
        return;
    }

    auto* model = mSessionManager->getNotificationListModel(mNotificationListModelId);

    if (!model)
    {
        qWarning() << "No model:" << mProfile.getHandle();
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get notifications still in progress:" << mProfile.getHandle();
        return;
    }

    model->setGetFeedInProgress(true);

    mBsky->listNotifications(limit, Utils::makeOptionalString(cursor), {}, {}, {},
        [this, presence=getPresence(), cursor](auto ouput){
            if (!presence)
                return;

            auto* model = mSessionManager->getNotificationListModel(mNotificationListModelId);

            if (!model)
                return;

            const bool clearFirst = cursor.isEmpty();

            // The bsky client from the active user is passed in to get the post stats from
            // the active user on a post
            auto* activeUserBsky = mSessionManager->getActiveUserBskyClient();
            model->addNotifications(std::move(ouput), *activeUserBsky, clearFirst,
                [this, presence]{
                    if (!presence)
                        return;

                    auto* model = mSessionManager->getNotificationListModel(mNotificationListModelId);

                    if (!model)
                        return;

                    model->setGetFeedInProgress(false);
                });
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            auto* model = mSessionManager->getNotificationListModel(mNotificationListModelId);

            if (!model)
                return;

            model->setGetFeedInProgress(false);
            mSessionManager->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        },
        updateSeen);

    if (updateSeen)
    {
        model->setNotificationsSeen(true);
        mSessionManager->setUnreadNotificationCount(mProfile.getDid(), 0);
    }
}

void NonActiveUser::getNotificationsNextPage()
{
    auto* model = mSessionManager->getNotificationListModel(mNotificationListModelId);

    if (!model)
        return;

    if (!model)
    {
        qWarning() << "No model:" << mProfile.getHandle();
        return;
    }

    const QString& cursor = model->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, cursor);
}

PostView* NonActiveUser::getPostView() const
{
    return mPostView ? mPostView.get() : nullptr;
}

void NonActiveUser::setPostView(PostView::Ptr postView)
{
    mPostView = std::move(postView);
    emit postViewChanged();
}

void NonActiveUser::clearPostView()
{
    if (mPostView)
    {
        mPostView = nullptr;
        emit postViewChanged();
    }
}

void NonActiveUser::getPost(const QString& uri)
{
    qDebug() << "Get post:" << uri << "handle:" << mProfile.getHandle();
    Q_ASSERT(mBsky);

    if (!mBsky)
    {
        qWarning() << "No atproto client:" << mProfile.getHandle();
        return;
    }

    if (mGetPostInProgress)
    {
        qDebug() << "Get post still in progress";
        return;
    }

    setPostInProgress(true);

    mBsky->getPosts({uri},
        [this, presence=getPresence(), uri](ATProto::AppBskyFeed::PostView::List posts){
            if (!presence)
                return;

            setPostInProgress(false);

            if (posts.empty())
            {
                qWarning() << "Got no posts for:" << uri << "handle:" << mProfile.getHandle();
                auto notFound = std::make_unique<Post>(Post::createNotFound(uri));
                auto notFoundView = std::make_unique<PostView>(std::move(notFound));
                setPostView(std::move(notFoundView));
                return;
            }

            // TODO: check if blocked and notFound posts have uri set
            auto post = std::make_unique<Post>(posts.front());
            auto postView = std::make_unique<PostView>(std::move(post));
            qDebug() << "Post view:" << postView->getUri() << postView->isGood();
            setPostView(std::move(postView));
        },
        [this, presence=getPresence(), uri](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Get post failed:" << error << "-" << msg << "handle:" << mProfile.getHandle();
            setPostInProgress(false);
            auto errorView = std::make_unique<PostView>(uri, msg);
            setPostView(std::move(errorView));
        });
}

void NonActiveUser::like(const QString& viaUri, const QString& viaCid)
{
    qDebug() << "Like by:" << mProfile.getHandle();

    if (!mPostView || !mPostView->isGood())
    {
        qWarning() << "No post to like:" << mProfile.getHandle();
        return;
    }

    if (!postMaster())
        return;

    if (isActionInProgress())
    {
        qDebug() << "Action still in progress";
        return;
    }

    if (mPostView->getLikeUri().isEmpty())
        doLike(viaUri, viaCid);
    else
        undoLike();
}

void NonActiveUser::doLike(const QString& viaUri, const QString& viaCid)
{
    const auto uri = mPostView->getUri();
    const auto cid = mPostView->getCid();

    setActionInProgress(true);

    postMaster()->like(uri, cid, viaUri, viaCid,
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            if (!presence)
                return;

            setActionInProgress(false);

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [cid, likeUri](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, 1);
                });

            if (mPostView)
                mPostView->setLikeUri(likeUri);
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::undoLike()
{
    const auto cid = mPostView->getCid();
    const auto likeUri = mPostView->getLikeUri();

    setActionInProgress(true);

    postMaster()->undo(likeUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            setActionInProgress(false);

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [cid](LocalPostModelChanges* model){
                    model->updateLikeCountDelta(cid, -1);
                });

            if (mPostView)
                mPostView->setLikeUri("");
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::setPostInProgress(bool inProgress)
{
    if (inProgress != mGetPostInProgress)
    {
        mGetPostInProgress = inProgress;
        emit getPostInProgressChanged();
    }
}

void NonActiveUser::setActionInProgress(bool inProgress)
{
    if (inProgress != mActionInProgress)
    {
        mActionInProgress = inProgress;
        emit actionInProgressChanged();
    }
}

ATProto::PostMaster* NonActiveUser::postMaster()
{
    if (!mPostMaster)
    {
        Q_ASSERT(mBsky);

        if (mBsky)
            mPostMaster = std::make_unique<ATProto::PostMaster>(*mBsky);
        else
            qWarning() << "Bsky client missing";
    }

    return mPostMaster.get();
}

}
