// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "non_active_user.h"
#include "session_manager.h"
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

}
