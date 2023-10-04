// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "local_post_model_changes.h"
#include "notification.h"
#include "post_cache.h"
#include <atproto/lib/client.h>
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class NotificationListModel : public QAbstractListModel, public LocalPostModelChanges
{
    Q_OBJECT
public:
    enum class Role {
        NotificationAuthor = Qt::UserRole + 1,
        NotificationOtherAuthors,
        NotificationReason,
        NotificationReasonSubjectUri,
        NotificationReasonSubjectCid,
        NotificationReasonPostText,
        NotificationReasonPostPlainText,
        NotificationReasonPostTimestamp,
        NotificationReasonPostImages,
        NotificationReasonPostExternal,
        NotificationReasonPostRecord,
        NotificationReasonPostRecordWithMedia,
        NotificationReasonPostNotFound,
        NotificationReasonPostLocallyDeleted,
        NotificationTimestamp,
        NotificationIsRead,
        NotificationPostUri,
        NotificationCid,
        NotificationPostText,
        NotificationPostPlainText,
        NotificationPostTimestamp,
        NotificationPostImages,
        NotificationPostExternal,
        NotificationPostRecord,
        NotificationPostRecordWithMedia,
        NotificationPostReplyRootUri,
        NotificationPostReplyRootCid,
        NotificationPostRepostUri,
        NotificationPostLikeUri,
        NotificationPostRepostCount,
        NotificationPostLikeCount,
        NotificationPostReplyCount,
        NotificationPostNotFound,
        ReplyToAuthor,
        EndOfList
    };

    explicit NotificationListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::Ptr notifications,
                          ATProto::Client& bsky, bool clearFirst = false);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Q_INVOKABLE bool notificationsLoaded() const { return !mList.empty(); }

protected:
    virtual void postIndexTimestampChanged() override;
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void replyCountChanged() override;
    virtual void repostCountChanged() override;
    virtual void repostUriChanged() override;
    virtual void postDeletedChanged() override;

    QHash<int, QByteArray> roleNames() const override;

private:
    using NotificationList = std::deque<Notification>;

    NotificationList createNotificationList(const ATProto::AppBskyNotification::NotificationList& rawList) const;
    void addNotificationList(const NotificationList& list, bool clearFirst);

    // Get the posts for LIKE, FOLLOW and REPOST notifications
    void getPosts(ATProto::Client& bsky, const NotificationList& list, const std::function<void()>& cb);
    void getPosts(ATProto::Client& bsky, std::unordered_set<QString> uris, const std::function<void()>& cb);

    void changeData(const QList<int>& roles);
    void clearLocalState();
    void clearRows();

    NotificationList mList;
    std::vector<ATProto::AppBskyNotification::ListNotificationsOutput::Ptr> mRawNotifications;
    QString mCursor;

    // This cache must be emptied when the notifications are refreshed, because
    // the counts (like, reposts, replies) will change over time and are displayed.
    PostCache mPostCache;

    // Posts in this cache can be kept for a long time
    PostCache mReasonPostCache;
};

}
