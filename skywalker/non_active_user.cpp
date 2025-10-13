// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "non_active_user.h"
#include "session_manager.h"
#include "utils.h"

namespace Skywalker {

static constexpr int NOTIFICATIONS_ADD_PAGE_SIZE = 50;

NonActiveUser::NonActiveUser(const BasicProfile& profile, bool sessionExpired, NotificationListModel* notificationListModel,
                             ATProto::Client* bsky, SessionManager* sessionManager, QObject* parent) :
    QObject(parent),
    mProfile(profile),
    mSessionExpired(sessionExpired),
    mNotificationListModel(notificationListModel),
    mBsky(bsky),
    mSessionManager(sessionManager)
{
    Q_ASSERT(mSessionManager);

    if (mNotificationListModel)
        mNotificationListModel->setParent(this);
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

    if (!mNotificationListModel)
    {
        qWarning() << "No model:" << mProfile.getHandle();
        return;
    }

    if (mNotificationListModel->isGetFeedInProgress())
    {
        qDebug() << "Get notifications still in progress:" << mProfile.getHandle();
        return;
    }

    mNotificationListModel->setGetFeedInProgress(true);

    mBsky->listNotifications(limit, Utils::makeOptionalString(cursor), {}, {}, {},
        [this, cursor](auto ouput){
            const bool clearFirst = cursor.isEmpty();

            // The bsky client from the active user is passed in to get the post stats from
            // the active user on a post
            auto* activeUserBsky = mSessionManager->getActiveUserBskyClient();
            mNotificationListModel->addNotifications(std::move(ouput), *activeUserBsky, clearFirst,
                [this]{
                    mNotificationListModel->setGetFeedInProgress(false);
                });
        },
        [this](const QString& error, const QString& msg){
            qDebug() << "getNotifications FAILED:" << error << " - " << msg << "hanlde:" << mProfile.getHandle();
            mNotificationListModel->setGetFeedInProgress(false);
            // TODO set feed error
        },
        updateSeen);

    if (updateSeen)
    {
        mNotificationListModel->setNotificationsSeen(true);
        mSessionManager->setUnreadNotificationCount(mProfile.getDid(), 0);
    }
}

void NonActiveUser::getNotificationsNextPage()
{
    if (!mNotificationListModel)
    {
        qWarning() << "No model:" << mProfile.getHandle();
        return;
    }

    const QString& cursor = mNotificationListModel->getCursor();

    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getNotifications(NOTIFICATIONS_ADD_PAGE_SIZE, false, cursor);
}

}
