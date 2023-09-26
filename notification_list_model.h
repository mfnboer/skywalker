// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "notification.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class NotificationListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        NotificationAuthor = Qt::UserRole + 1,
        NotificationOtherAuthors,
        NotificationReason,
        NotificationReasonSubjectUri,
        NotificationTimestamp,
        NotificationIsRead,
        NotificationPostText,
        ReplyToAuthor,
        EndOfList
    };

    explicit NotificationListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::Ptr notifications);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Q_INVOKABLE bool notificationsLoaded() const { return !mList.empty(); }

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    using NotificationList = std::deque<Notification>;

    NotificationList createNotifcationList(const ATProto::AppBskyNotification::NotificationList& rawList) const;

    NotificationList mList;
    std::vector<ATProto::AppBskyNotification::ListNotificationsOutput::Ptr> mRawNotifications;
    QString mCursor;
};

}
