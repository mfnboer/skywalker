// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification_list_model.h"
#include "abstract_post_feed_model.h"
#include "enums.h"
#include <atproto/lib/at_uri.h>
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
    mPostCache.clear();
    clearLocalChanges();
    endRemoveRows();
}

void NotificationListModel::addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::Ptr notifications, ATProto::Client& bsky)
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

    const auto notificationList = createNotificationList(notifications->mNotifications);
    mRawNotifications.push_back(std::move(notifications));

    getPosts(bsky, notificationList, [this, notificationList]{
        addNotificationList(notificationList);
    });
}

void NotificationListModel::addNotificationList(const NotificationList& list)
{
    const size_t newRowCount = mList.size() + list.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), list.begin(), list.cend());
    endInsertRows();

    qDebug() << "New list size:" << mList.size();
}

NotificationListModel::NotificationList NotificationListModel::createNotificationList(const ATProto::AppBskyNotification::NotificationList& rawList) const
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

void NotificationListModel::getPosts(ATProto::Client& bsky, const NotificationList& list, const std::function<void()>& cb)
{
    std::unordered_set<QString> uris;

    for (const auto& notification : list)
    {
        switch (notification.getReason())
        {
        case Notification::Reason::LIKE:
        case Notification::Reason::REPOST:
        {
            const auto& uri = notification.getReasonSubjectUri();

            if (ATProto::ATUri(uri).isValid() && !mReasonPostCache.contains(uri))
                uris.insert(uri);

            break;
        }
        case Notification::Reason::REPLY:
        case Notification::Reason::MENTION:
        case Notification::Reason::QUOTE:
        {
            const auto& uri = notification.getUri();

            if (ATProto::ATUri(uri).isValid() && !mPostCache.contains(uri))
                uris.insert(uri);

            break;
        }
        default:
            break;
        }
    }

    getPosts(bsky, uris, cb);
}

void NotificationListModel::getPosts(ATProto::Client& bsky, std::unordered_set<QString> uris, const std::function<void()>& cb)
{
    if (uris.empty())
    {
        cb();
        return;
    }

    std::vector<QString> uriList(uris.begin(), uris.end());

    if (uriList.size() > bsky.MAX_URIS_GET_POSTS)
    {
        uriList.resize(bsky.MAX_URIS_GET_POSTS);

        for (const auto& uri : uriList)
            uris.erase(uri);
    }
    else
    {
        uris.clear();
    }

    bsky.getPosts(uriList,
        [this, &bsky, uris, cb](auto postViewList)
        {
            for (auto& postView : postViewList)
            {
                // Store post view in both caches. The post cache will be cleared
                // on refresh.
                Post post(postView.get(), -1);
                ATProto::AppBskyFeed::PostView::SharedPtr sharedRaw(postView.release());
                mPostCache.put(sharedRaw, post);
                mReasonPostCache.put(sharedRaw, post);
            }

            getPosts(bsky, uris, cb);
        },
        [this, cb](const QString& err)
        {
            qWarning() << "Failed to get posts:" << err;
            cb();
        });
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
    const auto* change = getLocalChange(notification.getCid());

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
    case Role::NotificationReasonSubjectCid:
        return notification.getReasonPost(mReasonPostCache).getCid();
    case Role::NotificationReasonPostText:
        return notification.getReasonPost(mReasonPostCache).getFormattedText();
    case Role::NotificationReasonPostImages:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        QList<ImageView> images;
        for (const auto& img : post.getImages())
            images.push_back(*img);

        return QVariant::fromValue(images);
    }
    case Role::NotificationReasonPostExternal:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant();
    }
    case Role::NotificationReasonPostRecord:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        auto record = post.getRecordView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::NotificationReasonPostRecordWithMedia:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        auto record = post.getRecordWithMediaView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::NotificationReasonPostTimestamp:
        return notification.getReasonPost(mReasonPostCache).getTimelineTimestamp();
    case Role::NotificationReasonPostNotFound:
        return notification.getReasonPost(mReasonPostCache).isNotFound();
    case Role::NotificationTimestamp:
        return notification.getTimestamp();
    case Role::NotificationIsRead:
        return notification.isRead();
    case Role::NotificationPostUri:
        return notification.getPostUri();
    case Role::NotificationCid:
        return notification.getCid();
    case Role::NotificationPostText:
        return notification.getPostRecord().getFormattedText();
    case Role::NotificationPostTimestamp:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        return post.isNotFound() ? notification.getTimestamp() : post.getTimelineTimestamp();
    }
    case Role::NotificationPostImages:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        QList<ImageView> images;
        for (const auto& img : post.getImages())
            images.push_back(*img);

        return QVariant::fromValue(images);
    }
    case Role::NotificationPostExternal:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant();
    }
    case Role::NotificationPostRecord:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto record = post.getRecordView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::NotificationPostRecordWithMedia:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto record = post.getRecordWithMediaView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::NotificationPostReplyRootUri:
        return notification.getPostRecord().getReplyRootUri();
    case Role::NotificationPostReplyRootCid:
        return notification.getPostRecord().getReplyRootCid();
    case Role::NotificationPostRepostUri:
        return change && change->mRepostUri ? *change->mRepostUri : notification.getNotificationPost(mPostCache).getRepostUri();
    case Role::NotificationPostLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : notification.getNotificationPost(mPostCache).getLikeUri();
    case Role::NotificationPostRepostCount:
        return notification.getNotificationPost(mPostCache).getRepostCount() + (change ? change->mRepostCountDelta : 0);
    case Role::NotificationPostLikeCount:
        return notification.getNotificationPost(mPostCache).getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::NotificationPostReplyCount:
        return notification.getNotificationPost(mPostCache).getReplyCount() + (change ? change->mReplyCountDelta : 0);
    case Role::NotificationPostNotFound:
        return notification.getNotificationPost(mPostCache).isNotFound();
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
        { int(Role::NotificationReasonSubjectCid), "notificationReasonSubjectCid" },
        { int(Role::NotificationReasonPostText), "notificationReasonPostText" },
        { int(Role::NotificationReasonPostImages), "notificationReasonPostImages" },
        { int(Role::NotificationReasonPostTimestamp), "notificationReasonPostTimestamp" },
        { int(Role::NotificationReasonPostExternal), "notificationReasonPostExternal" },
        { int(Role::NotificationReasonPostRecord), "notificationReasonPostRecord" },
        { int(Role::NotificationReasonPostRecordWithMedia), "notificationReasonPostRecordWithMedia" },
        { int(Role::NotificationReasonPostNotFound), "notificationReasonPostNotFound" },
        { int(Role::NotificationTimestamp), "notificationTimestamp" },
        { int(Role::NotificationIsRead), "notificationIsRead" },
        { int(Role::NotificationPostUri), "notificationPostUri" },
        { int(Role::NotificationCid), "notificationCid" },
        { int(Role::NotificationPostText), "notificationPostText" },
        { int(Role::NotificationPostTimestamp), "notificationPostTimestamp" },
        { int(Role::NotificationPostImages), "notificationPostImages" },
        { int(Role::NotificationPostExternal), "notificationPostExternal" },
        { int(Role::NotificationPostRecord), "notificationPostRecord" },
        { int(Role::NotificationPostRecordWithMedia), "notificationPostRecordWithMedia" },
        { int(Role::NotificationPostReplyRootUri), "notificationPostReplyRootUri" },
        { int(Role::NotificationPostReplyRootCid), "notificationPostReplyRootCid" },
        { int(Role::NotificationPostRepostUri), "notificationPostRepostUri" },
        { int(Role::NotificationPostLikeUri), "notificationPostLikeUri" },
        { int(Role::NotificationPostRepostCount), "notificationPostRepostCount" },
        { int(Role::NotificationPostLikeCount), "notificationPostLikeCount" },
        { int(Role::NotificationPostReplyCount), "notificationPostReplyCount" },
        { int(Role::NotificationPostNotFound), "notificationPostNotFound" },
        { int(Role::ReplyToAuthor), "replyToAuthor" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void NotificationListModel::postIndexTimestampChanged()
{
    changeData({ int(Role::NotificationTimestamp) });
}

void NotificationListModel::likeCountChanged()
{
    changeData({ int(Role::NotificationPostLikeCount) });
}

void NotificationListModel::likeUriChanged()
{
    changeData({ int(Role::NotificationPostLikeUri) });
}

void NotificationListModel::replyCountChanged()
{
    changeData({ int(Role::NotificationPostReplyCount) });
}

void NotificationListModel::repostCountChanged()
{
    changeData({ int(Role::NotificationPostRepostCount) });
}

void NotificationListModel::repostUriChanged()
{
    changeData({ int(Role::NotificationPostRepostUri) });
}

void NotificationListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
