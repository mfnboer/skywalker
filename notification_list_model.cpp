// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification_list_model.h"
#include "enums.h"

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

    const size_t newRowCount = mList.size() + notifications->mNotifications.size();

    // TODO: collapse similar notifications into one.
    beginInsertRows({}, mList.size(), newRowCount - 1);
    for (const auto& rawNotification : notifications->mNotifications)
    {
        Notification notification(rawNotification.get());
        mList.push_back(notification);
    }
    endInsertRows();

    mRawNotifications.push_back(std::move(notifications));
    qDebug() << "New list size:" << mList.size();
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
        { int(Role::NotificationReason), "notificationReason" },
        { int(Role::NotificationReasonSubjectUri), "notificationReasonSubjectUri" },
        { int(Role::NotificationTimestamp), "notificationTimestamp" },
        { int(Role::NotificationIsRead), "notificationIsRead" },
        { int(Role::NotificationPostText), "notificationPostText" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}
