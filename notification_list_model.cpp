// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification_list_model.h"
#include "abstract_post_feed_model.h"
#include "enums.h"
#include <unordered_map>

namespace Skywalker {

NotificationListModel::NotificationListModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

void NotificationListModel::clear()
{
    beginRemoveRows({}, 0, mList.size() - 1);
    mList.clear();
    mCursor.clear();
    endRemoveRows();
}

void NotificationListModel::addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::Ptr notifications)
{
    qDebug() << "Add notifications:" << notifications->mNotifications.size();

    mCursor = notifications->mCursor.value_or(QString());
    if (isEndOfList() && !mList.empty())
        mList.back().setEndOfList(true);

    if (notifications->mNotifications.empty())
    {
        qWarning() << "No notifications!";
        return;
    }

    const auto notificationList = createNotifcationList(notifications->mNotifications);
    const size_t newRowCount = mList.size() + notificationList.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), notificationList.begin(), notificationList.cend());
    endInsertRows();

    mRawNotifications.push_back(std::move(notifications));
    qDebug() << "New list size:" << mList.size();
}

NotificationListModel::NotificationList NotificationListModel::createNotifcationList(const ATProto::AppBskyNotification::NotificationList& rawList) const
{
    NotificationList notifications;
    std::unordered_map<Notification::Reason, std::unordered_map<QString, int>> aggregate;

    for (const auto& rawNotification : rawList)
    {
        Notification notification(rawNotification.get());

        switch (notification.getReason())
        {
        case Notification::Reason::LIKE:
        case Notification::Reason::FOLLOW:
        case Notification::Reason::REPOST:
        {
            const auto& uri = notification.getReasonSubjectUri();
            qDebug() << "SUBJECT URI:" << uri << "REASON:" << int(notification.getReason());

            auto& aggregateMap = aggregate[notification.getReason()];
            auto it = aggregateMap.find(uri);

            if (it != aggregateMap.end())
            {
                auto& aggregateNotification = notifications[it->second];
                aggregateNotification.addOtherAuthor(notification.getAuthor());
            }
            else
            {
                notifications.push_back(notification);
                aggregateMap[uri] = notifications.size() - 1;
            }

            break;
        }
        default:
            notifications.push_back(notification);
            break;
        }

        const auto& author = rawNotification->mAuthor;
        const BasicProfile authorProfile(author->mHandle, author->mDisplayName.value_or(""));
        AbstractPostFeedModel::cacheAuthorProfile(author->mDid, authorProfile);
    }

    return notifications;
}

int NotificationListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mList.size();
}

QVariant NotificationListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mList.size())
        return {};

    const auto& notification = mList[index.row()];

    switch (Role(role))
    {
    case Role::NotificationAuthor:
        return QVariant::fromValue(notification.getAuthor());
    case Role::NotificationOtherAuthors:
        return QVariant::fromValue(notification.getOtherAuthors());
    case Role::NotificationReason:
        return static_cast<QEnums::NotificationReason>(int(notification.getReason()));
    case Role::NotificationReasonSubjectUri:
        return notification.getReasonSubjectUri();
    case Role::NotificationTimestamp:
        return notification.getTimestamp();
    case Role::NotificationIsRead:
        return notification.isRead();
    case Role::NotificationPostText:
        return notification.getPostRecord().getFormattedText();
    case Role::ReplyToAuthor:
        return QVariant::fromValue(notification.getPostRecord().getReplyToAuthor());
    case Role::EndOfList:
        return notification.isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

QHash<int, QByteArray> NotificationListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::NotificationAuthor), "notificationAuthor" },
        { int(Role::NotificationOtherAuthors), "notificationOtherAuthors" },
        { int(Role::NotificationReason), "notificationReason" },
        { int(Role::NotificationReasonSubjectUri), "notificationReasonSubjectUri" },
        { int(Role::NotificationTimestamp), "notificationTimestamp" },
        { int(Role::NotificationIsRead), "notificationIsRead" },
        { int(Role::NotificationPostText), "notificationPostText" },
        { int(Role::ReplyToAuthor), "replyToAuthor" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}
