// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "non_active_user.h"
#include "post_utils.h"
#include "session_manager.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 50;

NonActiveUser::NonActiveUser(QObject* parent) :
    QObject(parent),
    mSessionExpired(true)
{
}

NonActiveUser::NonActiveUser(const BasicProfile& profile, bool sessionExpired,
                             ATProto::Client::SharedPtr bsky, SessionManager* sessionManager,
                             QObject* parent) :
    QObject(parent),
    mProfile(profile),
    mSessionExpired(sessionExpired),
    mBsky(bsky),
    mSessionManager(sessionManager)
{
    Q_ASSERT(mSessionManager);

    if (mBsky)
    {
        mSkywalker = sessionManager->getSkywalker()->createSkywalker(profile.getDid(), mBsky, this);
        mNotificationListModelId = mSkywalker->createNotificationListModel();
    }
}

NonActiveUser::~NonActiveUser()
{
    removeNotificationListModel();
}

void NonActiveUser::removeNotificationListModel()
{
    if (mNotificationListModelId >= 0)
    {
        mSkywalker->removeNotificationListModel(mNotificationListModelId);
        mNotificationListModelId = -1;
    }
}

void NonActiveUser::init()
{
    if (mSkywalker)
    {
        connect(mSkywalker.get(), &Skywalker::getUserPreferencesFailed, this,
                [this](QString error){
                    qWarning() << "Failed to get user preferences:" << error << "did:" << mProfile.getDid();

                    if (mSessionManager)
                        mSessionManager->deleteSession(mProfile.getDid());
                });

        mSkywalker->initNonActiveUser();
    }
}

void NonActiveUser::expireSession()
{
    if (mSessionExpired)
        return;

    setUnreadNotificationCount(0);
    removeNotificationListModel();

    mSessionExpired = true;
    mPostMaster = nullptr;
    mBsky = nullptr;

    emit sessionExpiredChanged();
}

Skywalker* NonActiveUser::getSkywalker() const
{
    return mSkywalker.get();
}

NotificationListModel* NonActiveUser::getNotificationListModel() const
{
    if (!mSkywalker)
        return nullptr;

    return mSkywalker->getNotificationListModel(mNotificationListModelId);
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

    auto* model = getNotificationListModel();

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

    mBsky->listNotifications(limit, Utils::makeOptionalString(cursor), {}, {},
        [this, presence=getPresence(), cursor](auto ouput){
            if (!presence)
                return;

            auto* model = getNotificationListModel();

            if (!model)
                return;

            const bool clearFirst = cursor.isEmpty();

            model->addNotifications(std::move(ouput), mBsky, clearFirst,
                [this, presence]{
                    if (!presence)
                        return;

                    auto* model = getNotificationListModel();

                    if (!model)
                        return;

                    model->setGetFeedInProgress(false);
                });
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            auto* model = getNotificationListModel();

            if (!model)
                return;

            model->setGetFeedInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
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
    auto* model = getNotificationListModel();

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

BasicProfileView* NonActiveUser::getProfileForAction() const
{
    return mProfileForAction ? mProfileForAction.get() : nullptr;
}

void NonActiveUser::setProfileForAction(BasicProfileView::Ptr profile)
{
    mProfileForAction = std::move(profile);
    emit profileForActionChanged();
}

void NonActiveUser::clearProfileForAction()
{
    if (mProfileForAction)
    {
        mProfileForAction = nullptr;
        emit profileForActionChanged();
    }
}

void NonActiveUser::clearObjectsForAction()
{
    clearPostView();
    clearProfileForAction();
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

    if (mGetActionObjectInProgress)
    {
        qDebug() << "Get still in progress";
        return;
    }

    setActionObjectInProgress(true);

    mBsky->getPosts({uri},
        [this, presence=getPresence(), uri](ATProto::AppBskyFeed::GetPostsOutput::SharedPtr output){
            if (!presence)
                return;

            setActionObjectInProgress(false);

            // If a post is blocked or deleted we simply do not receive the post
            if (output->mPosts.empty())
            {
                qWarning() << "Got no posts for:" << uri << "handle:" << mProfile.getHandle();
                auto notFound = std::make_unique<Post>(Post::createNotFound(uri));
                auto notFoundView = std::make_unique<PostView>(std::move(notFound));
                setPostView(std::move(notFoundView));
                return;
            }

            auto post = std::make_unique<Post>(output->mPosts.front());
            auto postView = std::make_unique<PostView>(std::move(post));
            qDebug() << "Post view:" << postView->getUri() << postView->isGood();
            setPostView(std::move(postView));
        },
        [this, presence=getPresence(), uri](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Get post failed:" << error << "-" << msg << "handle:" << mProfile.getHandle();
            setActionObjectInProgress(false);
            auto errorView = std::make_unique<PostView>(uri, msg);
            setPostView(std::move(errorView));
        });
}


void NonActiveUser::getProfileForAction(const QString& did)
{
    qDebug() << "Get profile:" << did << "handle:" << mProfile.getHandle();
    Q_ASSERT(mBsky);

    if (!mBsky)
    {
        qWarning() << "No atproto client:" << mProfile.getHandle();
        return;
    }

    if (mGetActionObjectInProgress)
    {
        qDebug() << "Get still in progress";
        return;
    }

    setActionObjectInProgress(true);

    mBsky->getProfile(did,
        [this, presence=getPresence(), did](ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr profile){
            if (!presence)
                return;

            setActionObjectInProgress(false);
            BasicProfile basicProfile(std::move(profile));
            auto view = std::make_unique<BasicProfileView>(basicProfile);
            qDebug() << "Profile for action:" << view->getDid() << "following:" << view->getFollowingUri();
            setProfileForAction(std::move(view));
        },
        [this, presence=getPresence(), did](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Get profile failed:" << error << "-" << msg << "handle:" << mProfile.getHandle();
            setActionObjectInProgress(false);
            auto errorView = std::make_unique<BasicProfileView>(did, msg);
            setProfileForAction(std::move(errorView));
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
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
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
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::repost(const QString& viaUri, const QString& viaCid)
{
    qDebug() << "Repost by:" << mProfile.getHandle();

    if (!mPostView || !mPostView->isGood())
    {
        qWarning() << "No post to repost:" << mProfile.getHandle();
        return;
    }

    if (!postMaster())
        return;

    if (isActionInProgress())
    {
        qDebug() << "Action still in progress";
        return;
    }

    if (mPostView->getRepostUri().isEmpty())
        doRepost(viaUri, viaCid);
    else
        undoRepost();
}

void NonActiveUser::doRepost(const QString& viaUri, const QString& viaCid)
{
    const auto uri = mPostView->getUri();
    const auto cid = mPostView->getCid();

    setActionInProgress(true);

    postMaster()->checkRecordExists(uri, cid,
        [this, presence=getPresence(), uri, cid, viaUri, viaCid]{
            if (presence)
                continueDoRepost(uri, cid, viaUri, viaCid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::continueDoRepost(const QString& uri, const QString& cid,const QString& viaUri, const QString& viaCid)
{
    if (!postMaster())
    {
        setActionInProgress(false);
        return;
    }

    postMaster()->repost(uri, cid, viaUri, viaCid,
        [this, presence=getPresence(), cid](const auto& repostUri, const auto&){
            if (!presence)
                return;

            setActionInProgress(false);

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [cid, repostUri](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(cid, 1);
                });

            if (mPostView)
                mPostView->setRepostUri(repostUri);

            mSessionManager->getSkywalker()->showStatusMessage(tr("Reposted"), QEnums::STATUS_LEVEL_INFO);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Repost failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::undoRepost()
{
    const auto origPostCid = mPostView->getCid();
    const auto repostUri = mPostView->getRepostUri();

    setActionInProgress(true);

    postMaster()->undo(repostUri,
        [this, presence=getPresence(), origPostCid]{
            if (!presence)
                return;

            setActionInProgress(false);

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [origPostCid](LocalPostModelChanges* model){
                    model->updateRepostCountDelta(origPostCid, -1);
                });

            if (mPostView)
                mPostView->setRepostUri("");

            mSessionManager->getSkywalker()->showStatusMessage(tr("Repost undone"), QEnums::STATUS_LEVEL_INFO);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo repost failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::bookmark()
{
    qDebug() << "Bookmark by:" << mProfile.getHandle();

    if (!mPostView || !mPostView->isGood())
    {
        qWarning() << "No post to bookmark:" << mProfile.getHandle();
        return;
    }

    if (!mBsky)
        return;

    if (isActionInProgress())
    {
        qDebug() << "Action still in progress";
        return;
    }

    if (!mPostView->isBookmarked())
        addBookmark();
    else
        removeBookmark();
}

void NonActiveUser::follow()
{
    qDebug() << "Follow by:" << mProfile.getHandle();

    if (!mProfileForAction || !mProfileForAction->isGood())
    {
        qWarning() << "No profile to follow:" << mProfile.getHandle();
        return;
    }

    if (mProfileForAction->getDid() == mProfile.getDid())
    {
        qWarning() << "Do not follow yourself:" << mProfile.getDid();
        return;
    }

    if (!graphMaster())
        return;

    if (isActionInProgress())
    {
        qDebug() << "Action still in progress";
        return;
    }

    if (mProfileForAction->getFollowingUri().isEmpty())
        doFollow();
    else
        undoFollow();
}

void NonActiveUser::addBookmark()
{
    const auto postUri = mPostView->getUri();
    const auto postCid = mPostView->getCid();

    setActionInProgress(true);

    mBsky->createBookmark(postUri, postCid,
        [this, presence=getPresence(), postUri, postCid]{
            qDebug() << "Bookmark added:" << postUri << "cid:" << postCid << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            setActionInProgress(false);

            if (mPostView)
                mPostView->setBookMarked(true);
        },
        [this, presence=getPresence(), postCid](const QString& error, const QString& msg){
            qWarning() << "Add bookmark failed:" << error << "-" << msg << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::removeBookmark()
{
    const auto postUri = mPostView->getUri();
    const auto postCid = mPostView->getCid();

    setActionInProgress(true);

    mBsky->deleteBookmark(postUri,
        [this, presence=getPresence(), postUri, postCid]{
            qDebug() << "Bookmark removed:" << postUri << "cid:" << postCid << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            setActionInProgress(false);

            if (mPostView)
                mPostView->setBookMarked(false);
        },
        [this, presence=getPresence(), postCid](const QString& error, const QString& msg){
            qWarning() << "Remove bookmark failed:" << error << "-" << msg << "handle:" << mProfile.getHandle();

            if (!presence)
                return;

            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::doFollow()
{
    const auto did = mProfileForAction->getDid();
    setActionInProgress(true);

    graphMaster()->follow(did,
        [this, presence=getPresence(), did](const auto& followingUri, const auto&){
            if (!presence)
                return;

            setActionInProgress(false);
            qDebug() << "Follow succeeded:" << did << "following:" << followingUri << "handle:" << mProfile.getHandle();

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [did, followingUri](LocalAuthorModelChanges* model){
                    model->updateFollowingUri(did, followingUri);
                });

            if (mProfileForAction)
                mProfileForAction->setFollowingUri(followingUri);
        },
        [this, presence=getPresence(), did](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Follow failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::undoFollow()
{
    const auto did = mProfileForAction->getDid();
    const auto followingUri = mProfileForAction->getFollowingUri();

    setActionInProgress(true);

    graphMaster()->undo(followingUri,
        [this, presence=getPresence(), did]{
            if (!presence)
                return;

            setActionInProgress(false);
            qDebug() << "Undo follw succeeded:" << did << "handle:" << mProfile.getHandle();

            mSessionManager->getSkywalker()->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateFollowingUri(did, "");
                });

            if (mProfileForAction)
                mProfileForAction->setFollowingUri("");
        },
        [this, presence=getPresence(), did](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo follow failed:" << error << " - " << msg << "handle:" << mProfile.getHandle();
            setActionInProgress(false);
            mSessionManager->getSkywalker()->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void NonActiveUser::setActionObjectInProgress(bool inProgress)
{
    if (inProgress != mGetActionObjectInProgress)
    {
        mGetActionObjectInProgress = inProgress;
        emit getActionObjectInProgressChanged();
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

ATProto::GraphMaster* NonActiveUser::graphMaster()
{
    if (!mGraphMaster)
    {
        Q_ASSERT(mBsky);

        if (mBsky)
            mGraphMaster = std::make_unique<ATProto::GraphMaster>(*mBsky);
        else
            qWarning() << "Bsky client missing";
    }

    return mGraphMaster.get();
}

}
