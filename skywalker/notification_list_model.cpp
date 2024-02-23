// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification_list_model.h"
#include "author_cache.h"
#include "content_filter.h"
#include "enums.h"
#include "invite_code_store.h"
#include <atproto/lib/at_uri.h>
#include <unordered_map>

namespace Skywalker {

NotificationListModel::NotificationListModel(const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                             const MutedWords& mutedWords, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter),
    mBookmarks(bookmarks),
    mMutedWords(mutedWords)
{
    connect(&mBookmarks, &Bookmarks::sizeChanged, this, [this]{ postBookmarkedChanged(); });
}

void NotificationListModel::clear()
{
    clearRows();
    clearLocalState();
    mInviteCodeUsedNotifications.clear();
}

void NotificationListModel::clearLocalState()
{
    mCursor.clear();
    mPostCache.clear();
    clearLocalChanges();
}

void NotificationListModel::clearRows()
{
    if (!mList.empty())
    {
        beginRemoveRows({}, 0, mList.size() - 1);
        mList.clear();
        endRemoveRows();
    }
}

void NotificationListModel::addInviteCodeUsageNotificationRows()
{
    if (mInviteCodeUsedNotifications.empty())
        return;

    beginInsertRows({}, mList.size(), mList.size() + mInviteCodeUsedNotifications.size() - 1);

    for (const auto& notification : mInviteCodeUsedNotifications)
        mList.push_back(notification);

    endInsertRows();
}


void NotificationListModel::addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::Ptr notifications,
                                             ATProto::Client& bsky, bool clearFirst)
{
    qDebug() << "Add notifications:" << notifications->mNotifications.size();

    // Keep the rows in place while retrieving posts from the network.
    // This avoids flashing of the notifications window while refreshing.
    // Erasing the cache may lead to NOT FOUND entries if the user scrolls
    // during this period. If that happens we can improve this. Sofar it
    // seems not needed.
    if (clearFirst)
        clearLocalState();

    mCursor = notifications->mCursor.value_or(QString());

    if (notifications->mNotifications.empty())
    {
        qWarning() << "No notifications!";

        if (clearFirst)
        {
            clearRows();
            addInviteCodeUsageNotificationRows();
        }

        if (isEndOfList() && !mList.empty())
        {
            mList.back().setEndOfList(true);
            const auto index = createIndex(mList.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfList) });
        }

        return;
    }

    auto notificationList = createNotificationList(notifications->mNotifications);
    mRawNotifications.push_back(std::move(notifications));

    getPosts(bsky, notificationList, [this, notificationList, clearFirst]{
        auto list = std::move(notificationList);
        filterNotificationList(list);
        addNotificationList(list, clearFirst);
    });
}

void NotificationListModel::filterNotificationList(NotificationList& list) const
{
    for (auto it = list.begin(); it != list.end();)
    {
        const auto& post = it->getNotificationPost(mPostCache);
        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(post.getLabels());

        if (visibility == QEnums::CONTENT_VISIBILITY_HIDE_POST)
        {
            qDebug() << "Hide post:" << post.getCid() << warning;
            it = list.erase(it);
        }
        else if (post.getAuthor().getViewer().isMuted())
        {
            qDebug() << "Muted author:" << post.getAuthor().getHandleOrDid() << post.getCid();
            it = list.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void NotificationListModel::addNotificationList(const NotificationList& list, bool clearFirst)
{
    if (clearFirst)
    {
        clearRows();
        addInviteCodeUsageNotificationRows();
    }

    const size_t newRowCount = mList.size() + list.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), list.begin(), list.cend());

    if (isEndOfList())
        mList.back().setEndOfList(true);

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

        const BasicProfile author(rawNotification->mAuthor.get());
        AuthorCache::instance().put(author);
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
        [cb](const QString& err, const QString& msg)
        {
            qWarning() << "Failed to get posts:" << err << " - " << msg;
            cb();
        });
}

void NotificationListModel::addInviteCodeUsageNofications(InviteCodeStore* inviteCodeStore)
{
    Q_ASSERT(inviteCodeStore);
    const auto& usedCodes = inviteCodeStore->getUsedSincePreviousSignIn();

    for (auto* code : usedCodes)
    {
        BasicProfile usedBy = code->getUsedBy();

        if (usedBy.isNull())
        {
            usedBy = BasicProfile(code->getUsedByDid(), "", "", "");
            connect(code, &InviteCode::usedByChanged, this, [this, code]{
                updateInviteCodeUser(code->getUsedBy()); });
        }

        Notification notification(code->getCode(), usedBy);
        mInviteCodeUsedNotifications.push_back(notification);
    }
}

void NotificationListModel::updateInviteCodeUser(const BasicProfile& profile)
{
    for (auto& notification : mInviteCodeUsedNotifications)
    {
        if (notification.getInviteCodeUsedBy().getDid() == profile.getDid())
            notification.setInviteCodeUsedBy(profile);
    }

    changeData({ int(Role::NotificationInviteCodeUsedBy) });
}

void NotificationListModel::dismissInviteCodeUsageNotification(int index)
{
    if (index < 0 || index >= (int)mInviteCodeUsedNotifications.size())
        return;

    mInviteCodeUsedNotifications.erase(mInviteCodeUsedNotifications.begin() + index);

    if (index >= (int)mList.size())
        return;

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    endRemoveRows();
}

int NotificationListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mList.size();
}

QVariant NotificationListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& notification = mList[index.row()];
    const auto* change = getLocalChange(notification.getCid());
    const auto* reasonChange = getLocalChange(notification.getReasonPost(mReasonPostCache).getCid());

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
    case Role::NotificationReasonPostPlainText:
        return notification.getReasonPost(mReasonPostCache).getText();
    case Role::NotificationReasonPostIsReply:
        return notification.getReasonPost(mReasonPostCache).isReply();
    case Role::NotificationReasonPostReplyToAuthor:
    {
        const auto author = notification.getReasonPost(mReasonPostCache).getReplyToAuthor();
        return author ? QVariant::fromValue(*author) : QVariant();
    }
    case Role::NotificationReasonPostImages:
        return QVariant::fromValue(notification.getReasonPost(mReasonPostCache).getImages());
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

        if (record)
        {
            const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record->getLabels());
            record->setContentVisibility(visibility);
            record->setContentWarning(warning);
            record->setMutedReason(mMutedWords);
            return QVariant::fromValue(*record);
        }

        return QVariant();
    }
    case Role::NotificationReasonPostRecordWithMedia:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        auto recordWithMedia = post.getRecordWithMediaView();

        if (!recordWithMedia)
            return QVariant();

        auto& record = recordWithMedia->getRecord();
        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record.getLabels());
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setMutedReason(mMutedWords);
        return QVariant::fromValue(*recordWithMedia);
    }
    case Role::NotificationReasonPostTimestamp:
        return notification.getReasonPost(mReasonPostCache).getTimelineTimestamp();
    case Role::NotificationReasonPostNotFound:
        return notification.getReasonPost(mReasonPostCache).isNotFound();
    case Role::NotificationReasonPostLabels:
        return QVariant::fromValue(ContentFilter::getContentLabels(notification.getReasonPost(mReasonPostCache).getLabels()));
    case Role::NotificationReasonPostLocallyDeleted:
        return reasonChange ? reasonChange->mPostDeleted : false;
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
    case Role::NotificationPostPlainText:
        return notification.getPostRecord().getText();
    case Role::NotificationPostTimestamp:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        return post.isNotFound() ? notification.getTimestamp() : post.getTimelineTimestamp();
    }
    case Role::NotificationPostImages:
        return QVariant::fromValue(notification.getNotificationPost(mPostCache).getImages());
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

        if (record)
        {
            const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record->getLabels());
            record->setContentVisibility(visibility);
            record->setContentWarning(warning);
            record->setMutedReason(mMutedWords);
            return QVariant::fromValue(*record);
        }

        return QVariant();
    }
    case Role::NotificationPostRecordWithMedia:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto recordWithMedia = post.getRecordWithMediaView();

        if (!recordWithMedia)
            return QVariant();

        auto& record = recordWithMedia->getRecord();
        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record.getLabels());
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setMutedReason(mMutedWords);
        return QVariant::fromValue(*recordWithMedia);
    }
    case Role::NotificationPostReplyRootUri:
        return notification.getPostRecord().getReplyRootUri();
    case Role::NotificationPostReplyRootCid:
        return notification.getPostRecord().getReplyRootCid();
    case Role::NotificationPostRepostUri:
        return change && change->mRepostUri ? *change->mRepostUri : notification.getNotificationPost(mPostCache).getRepostUri();
    case Role::NotificationPostLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : notification.getNotificationPost(mPostCache).getLikeUri();
    case Role::NotificationPostReplyDisabled:
        return notification.getNotificationPost(mPostCache).isReplyDisabled();
    case Role::NotificationPostReplyRestriction:
        return notification.getNotificationPost(mPostCache).getReplyRestriction();
    case Role::NotificationPostRepostCount:
        return notification.getNotificationPost(mPostCache).getRepostCount() + (change ? change->mRepostCountDelta : 0);
    case Role::NotificationPostLikeCount:
        return notification.getNotificationPost(mPostCache).getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::NotificationPostReplyCount:
        return notification.getNotificationPost(mPostCache).getReplyCount() + (change ? change->mReplyCountDelta : 0);
    case Role::NotificationPostBookmarked:
        return mBookmarks.isBookmarked(notification.getPostUri());
    case Role::NotificationPostNotFound:
        return notification.getNotificationPost(mPostCache).isNotFound();
    case Role::NotificationPostLabels:
        return QVariant::fromValue(ContentFilter::getContentLabels(notification.getNotificationPost(mPostCache).getLabels()));
    case Role::NotificationPostContentVisibility:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto [visibility, _] = mContentFilter.getVisibilityAndWarning(post.getLabels());
        return visibility;
    }
    case Role::NotificationPostContentWarning:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto [_, warning] = mContentFilter.getVisibilityAndWarning(post.getLabels());
        return warning;
    }
    case Role::NotificationPostMutedReason:
    {
        if (notification.getAuthor().getViewer().isMuted())
            return QEnums::MUTED_POST_AUTHOR;

        const auto& post = notification.getNotificationPost(mPostCache);

        if (mMutedWords.match(post))
            return QEnums::MUTED_POST_WORDS;

        return QEnums::MUTED_POST_NONE;
    }
    case Role::NotificationPostIsReply:
        return notification.getPostRecord().isReply();
    case Role::ReplyToAuthor:
        return QVariant::fromValue(notification.getPostRecord().getReplyToAuthor());
    case Role::NotificationInviteCode:
        return notification.getInviteCode();
    case Role::NotificationInviteCodeUsedBy:
        return QVariant::fromValue(notification.getInviteCodeUsedBy());
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
        { int(Role::NotificationReasonPostPlainText), "notificationReasonPostPlainText" },
        { int(Role::NotificationReasonPostIsReply), "notificationReasonPostIsReply" },
        { int(Role::NotificationReasonPostReplyToAuthor), "notificationReasonPostReplyToAuthor" },
        { int(Role::NotificationReasonPostImages), "notificationReasonPostImages" },
        { int(Role::NotificationReasonPostTimestamp), "notificationReasonPostTimestamp" },
        { int(Role::NotificationReasonPostExternal), "notificationReasonPostExternal" },
        { int(Role::NotificationReasonPostRecord), "notificationReasonPostRecord" },
        { int(Role::NotificationReasonPostRecordWithMedia), "notificationReasonPostRecordWithMedia" },
        { int(Role::NotificationReasonPostNotFound), "notificationReasonPostNotFound" },
        { int(Role::NotificationReasonPostLabels), "notificationReasonPostLabels" },
        { int(Role::NotificationReasonPostLocallyDeleted), "notificationReasonPostLocallyDeleted" },
        { int(Role::NotificationTimestamp), "notificationTimestamp" },
        { int(Role::NotificationIsRead), "notificationIsRead" },
        { int(Role::NotificationPostUri), "notificationPostUri" },
        { int(Role::NotificationCid), "notificationCid" },
        { int(Role::NotificationPostText), "notificationPostText" },
        { int(Role::NotificationPostPlainText), "notificationPostPlainText" },
        { int(Role::NotificationPostTimestamp), "notificationPostTimestamp" },
        { int(Role::NotificationPostImages), "notificationPostImages" },
        { int(Role::NotificationPostExternal), "notificationPostExternal" },
        { int(Role::NotificationPostRecord), "notificationPostRecord" },
        { int(Role::NotificationPostRecordWithMedia), "notificationPostRecordWithMedia" },
        { int(Role::NotificationPostReplyRootUri), "notificationPostReplyRootUri" },
        { int(Role::NotificationPostReplyRootCid), "notificationPostReplyRootCid" },
        { int(Role::NotificationPostRepostUri), "notificationPostRepostUri" },
        { int(Role::NotificationPostLikeUri), "notificationPostLikeUri" },
        { int(Role::NotificationPostReplyDisabled), "notificationPostReplyDisabled" },
        { int(Role::NotificationPostReplyRestriction), "notificationPostReplyRestriction" },
        { int(Role::NotificationPostRepostCount), "notificationPostRepostCount" },
        { int(Role::NotificationPostLikeCount), "notificationPostLikeCount" },
        { int(Role::NotificationPostReplyCount), "notificationPostReplyCount" },
        { int(Role::NotificationPostBookmarked), "notificationPostBookmarked" },
        { int(Role::NotificationPostNotFound), "notificationPostNotFound" },
        { int(Role::NotificationPostLabels), "notificationPostLabels" },
        { int(Role::NotificationPostContentVisibility), "notificationPostContentVisibility" },
        { int(Role::NotificationPostContentWarning), "notificationPostContentWarning" },
        { int(Role::NotificationPostMutedReason), "notificationPostMutedReason" },
        { int(Role::NotificationPostIsReply), "notificationPostIsReply" },
        { int(Role::ReplyToAuthor), "replyToAuthor" },
        { int(Role::NotificationInviteCode), "notificationInviteCode" },
        { int(Role::NotificationInviteCodeUsedBy), "notificationInviteCodeUsedBy" },
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

void NotificationListModel::postDeletedChanged()
{
    changeData({ int(Role::NotificationReasonPostLocallyDeleted) });
}

void NotificationListModel::postBookmarkedChanged()
{
    changeData({ int(Role::NotificationPostBookmarked) });
}

void NotificationListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
