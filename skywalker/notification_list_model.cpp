// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification_list_model.h"
#include "author_cache.h"
#include "convo_view.h"
#include "content_filter.h"
#include "enums.h"
#include <atproto/lib/at_uri.h>
#include <unordered_map>

namespace Skywalker {

using namespace std::chrono_literals;

NotificationListModel::NotificationListModel(const ContentFilter& contentFilter,
                                             const MutedWords& mutedWords, FollowsActivityStore* followsActivityStore,
                                             QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter),
    mMutedWords(mutedWords),
    mFollowsActivityStore(followsActivityStore)
{
    connect(&AuthorCache::instance(), &AuthorCache::profileAdded, this,
            [this](const QString&){ changeData({ int(Role::ReplyToAuthor),
                                 int(Role::NotificationReasonPostReplyToAuthor),
                                 int(Role::NotificationPostRecord),
                                 int(Role::NotificationPostRecordWithMedia),
                                 int(Role::NotificationReasonPostRecord),
                                 int(Role::NotificationReasonPostRecordWithMedia),
                                 int(Role::NotificationPostContentLabeler) });
            });
}

void NotificationListModel::clear()
{
    clearRows();
    clearLocalState();
    mInviteCodeUsedNotifications.clear();
    mNewLabelsNotifications.clear();
    mNotificationsSeen = false;
    setPriority(false);
}

void NotificationListModel::clearLocalState()
{
    mCursor.clear();
    mPostCache.clear();
    clearLocalChanges();
    clearLocalProfileChanges();
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

void NotificationListModel::setPriority(bool priority)
{
    if (priority == mPriority)
        return;

    mPriority = priority;
    emit priorityChanged();
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

void NotificationListModel::addNewLabelsNotificationRows()
{
    updateNewLabelsNotifications();

    if (mNewLabelsNotifications.empty())
        return;

    beginInsertRows({}, mList.size(), mList.size() + mNewLabelsNotifications.size() - 1);

    for (const auto& notification : mNewLabelsNotifications)
        mList.push_back(notification);

    endInsertRows();
}

void NotificationListModel::updateNewLabelsNotifications()
{
    for (auto it = mNewLabelsNotifications.begin(); it != mNewLabelsNotifications.end(); )
    {
        if (!it->updateNewLabels(&mContentFilter))
            it = mNewLabelsNotifications.erase(it);
        else
            ++it;
    }
}

bool NotificationListModel::addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr notifications,
                                             ATProto::Client::SharedPtr bsky, bool clearFirst,
                                             const std::function<void()>& doneCb)
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
    setPriority(notifications->mPriority);

    if (notifications->mNotifications.empty())
    {
        qWarning() << "No notifications!";

        if (clearFirst)
        {
            clearRows();
            addInviteCodeUsageNotificationRows();
            addNewLabelsNotificationRows();
        }

        if (isEndOfList() && !mList.empty())
        {
            mList.back().setEndOfList(true);
            const auto index = createIndex(mList.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfList) });
        }

        if (doneCb)
            doneCb();

        return false;
    }

    auto notificationList = createNotificationList(notifications->mNotifications);

    getPosts(*bsky, notificationList, [this, notificationList, clearFirst, doneCb]{
        auto list = std::move(notificationList);
        filterNotificationList(list);
        addNotificationList(list, clearFirst);

        if (doneCb)
            doneCb();
    });

    return true;
}

static const QString& getConvoLastRevIncludingReactions(const ATProto::ChatBskyConvo::ConvoView& convo)
{
    if (!convo.mLastReaction)
    {
        qDebug() << "Convo rev:" << convo.mRev;
        return convo.mRev;
    }

    auto msgAndReaction = std::get<ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr>(*convo.mLastReaction);
    qDebug() << "Convo rev:" << convo.mRev << "reaction rev:" << msgAndReaction->mMessageView->mRev;
    return std::max(convo.mRev, msgAndReaction->mMessageView->mRev);
}

QString NotificationListModel::addNotifications(
    ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr convoListOutput, const QString& lastRev, const QString& userDid)
{
    qDebug() << "Add chat notifications:" << convoListOutput->mConvos.size() << "lastRev:" << lastRev;

    if (convoListOutput->mConvos.empty())
    {
        qDebug() << "No conversations";
        return lastRev;
    }

    QString newRev = lastRev;

    for (const auto& convo : convoListOutput->mConvos)
    {
        const QString& convoRev = getConvoLastRevIncludingReactions(*convo);

        if (convoRev <= lastRev)
        {
            qDebug() << "Messages already seen, rev:" << convoRev << "last:" << lastRev;
            continue;
        }

        qDebug() << "New convo rev:" << convoRev << "last:" << lastRev;
        newRev = std::max(newRev, convoRev);
        addConvoLastMessage(*convo, lastRev, userDid);
        addConvoLastReaction(*convo, lastRev, userDid);
    }

    return newRev;
}

void NotificationListModel::addConvoLastMessage(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& lastRev, const QString& userDid)
{
    if (!convo.mLastMessage)
    {
        qDebug() << "Convo has no last message:" << convo.mId << convo.mRev;
        return;
    }

    if (convo.mRev <= lastRev)
    {
        qDebug() << "Last message already seen:" << convo.mId << convo.mRev;
        return;
    }

    const MessageView messageView(*convo.mLastMessage);

    if (messageView.isDeleted())
    {
        qDebug() << "Last message is deleted";
        return;
    }

    const ConvoView convoView(convo, userDid);
    const ChatBasicProfile sender = convoView.getMember(convoView.getLastMessage().getSenderDid());
    const BasicProfile profile = sender.getBasicProfile();

    if (profile.isNull())
    {
        qWarning() << "Unknown message sender:" << convo.mId << convo.mRev;
        return;
    }

    const Notification notification(messageView, profile);
    mList.push_back(notification);
}

void NotificationListModel::addConvoLastReaction(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& lastRev, const QString& userDid)
{
    if (!convo.mLastReaction)
    {
        qDebug() << "Convo has no last reaction:" << convo.mId << convo.mRev;
        return;
    }

    auto msgAndReaction = std::get<ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr>(*convo.mLastReaction);

    if (msgAndReaction->mMessageView->mRev <= lastRev)
    {
        qDebug() << "Last reaction already seen:" << convo.mId << msgAndReaction->mMessageView->mRev;
        return;
    }

    const ConvoView convoView(convo, userDid);
    const ChatBasicProfile sender = convoView.getMember(msgAndReaction->mReactionView->mSender->mDid);
    const BasicProfile profile = sender.getBasicProfile();

    if (profile.isNull())
    {
        qWarning() << "Unknown reaction sender:" << convo.mId << convo.mRev;
        return;
    }

    const MessageAndReactionView messageAndReactionView(*convo.mLastReaction);
    const Notification notification(messageAndReactionView, profile);
    mList.push_back(notification);
}

void NotificationListModel::filterNotificationList(NotificationList& list) const
{
    for (auto it = list.begin(); it != list.end();)
    {
        const auto& post = it->getNotificationPost(mPostCache);
        const auto [visibility, warning, _] = mContentFilter.getVisibilityAndWarning(
            post.getAuthorDid(), post.getLabelsIncludingAuthorLabels());

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
        addNewLabelsNotificationRows();
    }

    const size_t newRowCount = mList.size() + list.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), list.begin(), list.cend());

    if (isEndOfList())
        mList.back().setEndOfList(true);

    endInsertRows();

    qDebug() << "New list size:" << mList.size();
}

void NotificationListModel::reportActivity(const Notification& notification) const
{
    if (!mFollowsActivityStore)
        return;

    const auto timestamp = notification.getTimestamp();

    if (!timestamp.isValid())
        return;

    const BasicProfile profile = notification.getAuthor();

    if (profile.isNull())
        return;

    mFollowsActivityStore->reportActivity(profile.getDid(), timestamp);
}

NotificationListModel::NotificationList NotificationListModel::createNotificationList(const ATProto::AppBskyNotification::Notification::List& rawList) const
{
    NotificationList notifications;
    std::unordered_map<Notification::Reason, std::unordered_map<QString, int>> aggregate;

    for (const auto& rawNotification : rawList)
    {
        Notification notification(rawNotification);
        reportActivity(notification);

        switch (notification.getReason())
        {
        case Notification::Reason::NOTIFICATION_REASON_LIKE:
        case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
        case Notification::Reason::NOTIFICATION_REASON_FOLLOW:
        case Notification::Reason::NOTIFICATION_REASON_REPOST:
        case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
        case Notification::Reason::NOTIFICATION_REASON_VERIFIED:
        case Notification::Reason::NOTIFICATION_REASON_UNVERIFIED:
        case Notification::Reason::NOTIFICATION_REASON_UNKNOWN:
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

        const BasicProfile author(rawNotification->mAuthor);
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
        case Notification::Reason::NOTIFICATION_REASON_LIKE:
        case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
        case Notification::Reason::NOTIFICATION_REASON_REPOST:
        case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
        {
            const auto& uri = notification.getReasonSubjectUri();

            if (ATProto::ATUri(uri).isValid() && !mReasonPostCache.contains(uri))
                uris.insert(uri);

            break;
        }
        case Notification::Reason::NOTIFICATION_REASON_REPLY:
        case Notification::Reason::NOTIFICATION_REASON_MENTION:
        case Notification::Reason::NOTIFICATION_REASON_QUOTE:
        case Notification::Reason::NOTIFICATION_REASON_SUBSCRIBED_POST:
            if (mRetrieveNotificationPosts)
            {
                const auto& uri = notification.getUri();

                if (ATProto::ATUri(uri).isValid() && !mPostCache.contains(uri))
                    uris.insert(uri);
            }

            break;
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
        if (cb)
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
                Post post(postView);
                mPostCache.put(post);
                mReasonPostCache.put(post);
            }

            getPosts(bsky, uris, cb);
        },
        [cb](const QString& err, const QString& msg)
        {
            qWarning() << "Failed to get posts:" << err << " - " << msg;

            if (cb)
                cb();
        });
}

// void NotificationListModel::addInviteCodeUsageNofications(InviteCodeStore* inviteCodeStore)
// {
//     Q_ASSERT(inviteCodeStore);
//     const auto& usedCodes = inviteCodeStore->getUsedSincePreviousSignIn();

//     for (auto* code : usedCodes)
//     {
//         BasicProfile usedBy = code->getUsedBy();

//         if (usedBy.isNull())
//         {
//             usedBy = BasicProfile(code->getUsedByDid(), "", "", "");
//             connect(code, &InviteCode::usedByChanged, this, [this, code]{
//                 updateInviteCodeUser(code->getUsedBy()); });
//         }

//         Notification notification(code->getCode(), usedBy);
//         mInviteCodeUsedNotifications.push_back(notification);
//     }
// }

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

void NotificationListModel::dismissNewLabelNotification(int index)
{
    qDebug() << "Dismiss new label notification:" << index;
    Q_ASSERT(mInviteCodeUsedNotifications.empty());

    if (index < 0 || index >= (int)mNewLabelsNotifications.size())
        return;

    {
        const auto& notification = mNewLabelsNotifications[index];
        mContentFilter.saveLabelIdsToSettings(notification.getLabelerDid());
        mNewLabelsNotifications.erase(mNewLabelsNotifications.begin() + index);
    }

    if (index >= (int)mList.size())
        return;

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    endRemoveRows();
}

int NotificationListModel::addNewLabelsNotifications(const std::unordered_map<QString, BasicProfile>& labelerProfiles)
{
    const std::unordered_map<QString, std::unordered_set<QString>>& labelerMap = mContentFilter.getLabelerDidsWithNewLabels();
    int notificationCount = 0;

    for (const auto& [labelerDid, labels] : labelerMap)
    {
        Q_ASSERT(labelerProfiles.contains(labelerDid));
        if (!labelerProfiles.contains(labelerDid))
        {
            qWarning() << "Profile for labeler missing:" << labelerDid;
            continue;
        }

        const BasicProfile& labeler = labelerProfiles.at(labelerDid);
        const Notification notification(labeler, labels);
        mNewLabelsNotifications.push_back(notification);
        ++notificationCount;
    }

    return notificationCount;
}

void NotificationListModel::setNotificationsSeen(bool seen)
{
    qDebug() << "Seen:" << seen;
    mNotificationsSeen = seen;
}

void NotificationListModel::updateRead()
{
    qDebug() << "Update read";

    for (auto& notification : mNewLabelsNotifications)
        notification.setIsRead(true);

    changeData({ int(Role::NotificationIsRead) });
}

int NotificationListModel::getIndexOldestUnread() const
{
    int index = -1;

    for (int i = 0; i < (int)mList.size(); ++i)
    {
        if (mList[i].isRead())
            break;

        index = i;
    }

    qDebug() << "Oldest unread index:" << index;
    return index;
}

QDateTime NotificationListModel::getTimestampLatestNotifcation() const
{
    if (mList.empty())
        return {};

    return mList.front().getTimestamp();
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
    case Role::NotificationAllAuthors:
        return QVariant::fromValue(notification.getAllAuthors());
    case Role::NotificationReason:
        return notification.getReason();
    case Role::NotificationReasonRaw:
        return notification.getRawReason();
    case Role::NotificationIsAggregatable:
        return notification.isAggregatable();
    case Role::NotificationReasonSubjectUri:
        return notification.getReasonSubjectUri();
    case Role::NotificationReasonSubjectCid:
        return notification.getReasonPost(mReasonPostCache).getCid();
    case Role::NotificationReasonPostText:
        return notification.getReasonPost(mReasonPostCache).getFormattedText();
    case Role::NotificationReasonPostPlainText:
        return notification.getReasonPost(mReasonPostCache).getText();
    case Role::NotificationReasonPostAuthor:
        return QVariant::fromValue(notification.getReasonPost(mReasonPostCache).getAuthor());
    case Role::NotificationReasonPostIsReply:
        return notification.getReasonPost(mReasonPostCache).isReply();
    case Role::NotificationReasonPostReplyToAuthor:
    {
        const auto post = notification.getReasonPost(mReasonPostCache);

        if (!post.isReply())
            return {};

        const auto author = post.getReplyToAuthor();

        if (!author)
        {
            const QString did = post.getReplyToAuthorDid();

            if (!did.isEmpty())
                AuthorCache::instance().putProfile(did);

            return {};
        }

        return QVariant::fromValue(*author);
    }
    case Role::NotificationReasonPostHasUnknownEmbed:
        return notification.getReasonPost(mReasonPostCache).hasUnknownEmbed();
    case Role::NotificationReasonPostUnknownEmbedType:
        return notification.getReasonPost(mReasonPostCache).getUnknownEmbedType();
    case Role::NotificationReasonPostImages:
        return QVariant::fromValue(notification.getReasonPost(mReasonPostCache).getImages());
    case Role::NotificationReasonPostVideo:
    {
        const auto& post = notification.getReasonPost(mReasonPostCache);
        auto video = post.getVideoView();
        return video ? QVariant::fromValue(*video) : QVariant{};
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

        if (record)
        {
            if (record->isReply())
            {
                const QString did = record->getReplyToAuthorDid();

                if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                    AuthorCache::instance().putProfile(did);
            }

            const auto labels = mContentFilter.getContentLabels(record->getLabels());
            const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
                record->getAuthorDid(), labels);
            record->setContentVisibility(visibility);
            record->setContentWarning(warning);
            record->setContentLabeler(getContentLabeler(visibility, labels, labelIndex));
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

        if (record.isReply())
        {
            const QString did = record.getReplyToAuthorDid();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                AuthorCache::instance().putProfile(did);
        }

        const auto labels = mContentFilter.getContentLabels(record.getLabels());
        const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
            record.getAuthorDid(), labels);
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setContentLabeler(getContentLabeler(visibility, labels, labelIndex));
        record.setMutedReason(mMutedWords);
        return QVariant::fromValue(*recordWithMedia);
    }
    case Role::NotificationReasonPostLanguages:
        return QVariant::fromValue(notification.getReasonPost(mReasonPostCache).getLanguages());
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
    case Role::NotificationSecondsAgo:
        return double((QDateTime::currentDateTimeUtc() - notification.getTimestamp()) / 1s);
    case Role::NotificationIsRead:
        return notification.isRead();
    case Role::NotificationPostUri:
        return notification.getPostUri();
    case Role::NotificationCid:
        return notification.getCid();
    case Role::NotificationLabels:
    {
        const QString did = notification.getLabelerDid();

        if (did.isEmpty())
            return QStringList{};

        QStringList labels;
        const auto& labelIds = notification.getLabelIds();

        for (const auto& label : labelIds)
        {
            auto* contentGroup = mContentFilter.getContentGroup(did, label);
            labels.push_back(contentGroup ? contentGroup->getTitle() : label);
        }

        return labels;
    }
    case Role::NotificationPostAuthor:
        return QVariant::fromValue(notification.getNotificationPost(mPostCache).getAuthor());
    case Role::NotificationPostText:
        return notification.getPostRecord().getFormattedText();
    case Role::NotificationPostPlainText:
        return notification.getPostRecord().getText();
    case Role::NotificationPostLanguages:
    {
        const auto& postRecord = notification.getPostRecord();
        return QVariant::fromValue(postRecord.getLanguages());
    }
    case Role::NotificationPostTimestamp:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        return post.isNotFound() ? notification.getTimestamp() : post.getTimelineTimestamp();
    }
    case Role::NotificationPostHasUnknownEmbed:
        return notification.getNotificationPost(mPostCache).hasUnknownEmbed();
    case Role::NotificationPostUnknownEmbedType:
        return notification.getNotificationPost(mPostCache).getUnknownEmbedType();
    case Role::NotificationPostImages:
        return QVariant::fromValue(notification.getNotificationPost(mPostCache).getImages());
    case Role::NotificationPostVideo:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto video = post.getVideoView();
        return video ? QVariant::fromValue(*video) : QVariant{};
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
        auto postRecord = post.getRecordView();
        RecordView* record = postRecord.get();

        if (!record)
            return QVariant();

        if (change)
        {
            if (change->mDetachedRecord)
                record = change->mDetachedRecord.get();
            else if (change->mReAttachedRecord)
                record = change->mReAttachedRecord.get();
        }

        if (record->isReply())
        {
            const QString did = record->getReplyToAuthorDid();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                AuthorCache::instance().putProfile(did);
        }

        const auto& labels = record->getLabelsIncludingAuthorLabels();
        const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
            record->getAuthorDid(), labels);
        record->setContentVisibility(visibility);
        record->setContentWarning(warning);
        record->setContentLabeler(getContentLabeler(visibility, labels, labelIndex));
        record->setMutedReason(mMutedWords);
        return QVariant::fromValue(*record);
    }
    case Role::NotificationPostRecordWithMedia:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        auto recordWithMedia = post.getRecordWithMediaView();

        if (!recordWithMedia)
            return QVariant();

        if (change)
        {
            if (change->mDetachedRecord)
                recordWithMedia->setRecord(change->mDetachedRecord);
            else if (change->mReAttachedRecord)
                recordWithMedia->setRecord(change->mReAttachedRecord);
        }

        auto& record = recordWithMedia->getRecord();

        if (record.isReply())
        {
            const QString did = record.getReplyToAuthorDid();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                AuthorCache::instance().putProfile(did);
        }

        const auto& labels = record.getLabelsIncludingAuthorLabels();
        const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
            record.getAuthorDid(), labels);
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setContentLabeler(getContentLabeler(visibility, labels, labelIndex));
        record.setMutedReason(mMutedWords);
        return QVariant::fromValue(*recordWithMedia);
    }
    case Role::NotificationPostReplyRootAuthorDid:
        return notification.getPostRecord().getReplyRootAuthorDid();
    case Role::NotificationPostReplyRootUri:
        return notification.getPostRecord().getReplyRootUri();
    case Role::NotificationPostReplyRootCid:
        return notification.getPostRecord().getReplyRootCid();
    case Role::NotificationPostMentionDids:
        return notification.getPostRecord().getMentionDids();
    case Role::NotificationPostRepostUri:
        return change && change->mRepostUri ? *change->mRepostUri : notification.getNotificationPost(mPostCache).getRepostUri();
    case Role::NotificationPostLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : notification.getNotificationPost(mPostCache).getLikeUri();
    case Role::NotificationPostLikeTransient:
        return change ? change->mLikeTransient : false;
    case Role::NotificationPostThreadMuted:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto* uriChange = getLocalUriChange(post.isReply() ? post.getReplyRootUri() : post.getUri());
        return uriChange && uriChange->mThreadMuted ? *uriChange->mThreadMuted : post.isThreadMuted();
    }
    case Role::NotificationPostReplyDisabled:
        return notification.getNotificationPost(mPostCache).isReplyDisabled();
    case Role::NotificationPostEmbeddingDisabled:
        return notification.getNotificationPost(mPostCache).isEmbeddingDisabled();
    case Role::NotificationPostViewerStatePinned:
        return change && change->mViewerStatePinned ? *change->mViewerStatePinned : notification.getNotificationPost(mPostCache).isViewerStatePinned();
    case Role::NotificationPostThreadgateUri:
    {
        const auto& post = notification.getNotificationPost(mPostCache);

        if (post.isReply())
            return change && change->mThreadgateUri ? *change->mThreadgateUri : post.getThreadgateUri();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mThreadgateUri ? *rootChange->mThreadgateUri : post.getThreadgateUri();
    }
    case Role::NotificationPostReplyRestriction:
    {
        const auto& post = notification.getNotificationPost(mPostCache);

        if (post.isReply())
            return change && change->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? change->mReplyRestriction : post.getReplyRestriction();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? rootChange->mReplyRestriction : post.getReplyRestriction();
    }
    case Role::NotificationPostReplyRestrictionLists:
    {
        const auto& post = notification.getNotificationPost(mPostCache);

        if (post.isReply())
            return QVariant::fromValue(change && change->mReplyRestrictionLists ? *change->mReplyRestrictionLists : post.getReplyRestrictionLists());

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return QVariant::fromValue(rootChange && rootChange->mReplyRestrictionLists ? *rootChange->mReplyRestrictionLists : post.getReplyRestrictionLists());
    }
    case Role::NotificationPostHiddenReplies:
    {
        const auto& post = notification.getNotificationPost(mPostCache);

        if (post.isReply())
            return change && change->mHiddenReplies ? *change->mHiddenReplies : post.getHiddenReplies();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mHiddenReplies ? *rootChange->mHiddenReplies : post.getHiddenReplies();
    }
    case Role::NotificationPostIsHiddenReply:
    {
        const auto& post = notification.getNotificationPost(mPostCache);

        if (post.isReply())
            return change && change->mHiddenReplies ? change->mHiddenReplies->contains(post.getUri()) : post.isHiddenReply();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mHiddenReplies ? rootChange->mHiddenReplies->contains(post.getUri()) : post.isHiddenReply();
    }
    case Role::NotificationPostRepostCount:
        return notification.getNotificationPost(mPostCache).getRepostCount() + (change ? change->mRepostCountDelta : 0);
    case Role::NotificationPostLikeCount:
        return notification.getNotificationPost(mPostCache).getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::NotificationPostQuoteCount:
        return notification.getNotificationPost(mPostCache).getQuoteCount() + (change ? change->mQuoteCountDelta : 0);
    case Role::NotificationPostReplyCount:
        return notification.getNotificationPost(mPostCache).getReplyCount() + (change ? change->mReplyCountDelta : 0);
    case Role::NotificationPostBookmarked:
        return change && change->mBookmarked ? *change->mBookmarked :  notification.getNotificationPost(mPostCache).isBookmarked();
    case Role::NotificationPostBookmarkTransient:
        return change ? change->mBookmarkTransient : false;
    case Role::NotificationPostNotFound:
        return notification.getNotificationPost(mPostCache).isNotFound();
    case Role::NotificationPostBlocked:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        return post.isBlocked() || getLocallyBlocked(post.getAuthor().getDid());
    }
    case Role::NotificationPostLabels:
        return QVariant::fromValue(ContentFilter::getContentLabels(notification.getNotificationPost(mPostCache).getLabels()));
    case Role::NotificationPostContentVisibility:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto [visibility, _, __] = mContentFilter.getVisibilityAndWarning(
            post.getAuthorDid(), post.getLabelsIncludingAuthorLabels());
        return visibility;
    }
    case Role::NotificationPostContentWarning:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto [_, warning, __] = mContentFilter.getVisibilityAndWarning(
            post.getAuthorDid(), post.getLabelsIncludingAuthorLabels());
        return warning;
    }
    case Role::NotificationPostContentLabeler:
    {
        const auto& post = notification.getNotificationPost(mPostCache);
        const auto& labels = post.getLabelsIncludingAuthorLabels();
        const auto [visibility, _, labelIndex] = mContentFilter.getVisibilityAndWarning(
            post.getAuthorDid(), labels);
        const auto labeler = getContentLabeler(visibility, labels, labelIndex);
        return QVariant::fromValue(labeler);
    }
    case Role::NotificationPostMutedReason:
    {
        if (notification.getAuthor().getViewer().isMuted())
            return QEnums::MUTED_POST_AUTHOR;

        const auto& post = notification.getNotificationPost(mPostCache);

        if (mMutedWords.match(post).first)
            return QEnums::MUTED_POST_WORDS;

        return QEnums::MUTED_POST_NONE;
    }
    case Role::NotificationPostIsReply:
        return notification.getPostRecord().isReply();
    case Role::ReplyToAuthor:
    {
        const auto record = notification.getPostRecord();

        if (!record.isReply())
            return {};

        const auto author = record.getReplyToAuthor();

        if (!author.isNull())
            return QVariant::fromValue(author);

        const QString did = record.getReplyToAuthorDid();

        if (!did.isEmpty() && !AuthorCache::instance().contains(did))
            AuthorCache::instance().putProfile(did);

        return {};
    }
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

void NotificationListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
}

QHash<int, QByteArray> NotificationListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::NotificationAuthor), "notificationAuthor" },
        { int(Role::NotificationOtherAuthors), "notificationOtherAuthors" },
        { int(Role::NotificationAllAuthors), "notificationAllAuthors" },
        { int(Role::NotificationReason), "notificationReason" },
        { int(Role::NotificationReasonRaw), "notificationReasonRaw" },
        { int(Role::NotificationIsAggregatable), "notificationIsAggregatable" },
        { int(Role::NotificationReasonSubjectUri), "notificationReasonSubjectUri" },
        { int(Role::NotificationReasonSubjectCid), "notificationReasonSubjectCid" },
        { int(Role::NotificationReasonPostText), "notificationReasonPostText" },
        { int(Role::NotificationReasonPostPlainText), "notificationReasonPostPlainText" },
        { int(Role::NotificationReasonPostAuthor), "notificationReasonPostAuthor" },
        { int(Role::NotificationReasonPostIsReply), "notificationReasonPostIsReply" },
        { int(Role::NotificationReasonPostReplyToAuthor), "notificationReasonPostReplyToAuthor" },
        { int(Role::NotificationReasonPostHasUnknownEmbed), "notificationReasonPostHasUnknownEmbed" },
        { int(Role::NotificationReasonPostUnknownEmbedType), "notificationReasonPostUnknownEmbedType" },
        { int(Role::NotificationReasonPostImages), "notificationReasonPostImages" },
        { int(Role::NotificationReasonPostVideo), "notificationReasonPostVideo" },
        { int(Role::NotificationReasonPostLanguages), "notificationReasonPostLanguages" },
        { int(Role::NotificationReasonPostTimestamp), "notificationReasonPostTimestamp" },
        { int(Role::NotificationReasonPostExternal), "notificationReasonPostExternal" },
        { int(Role::NotificationReasonPostRecord), "notificationReasonPostRecord" },
        { int(Role::NotificationReasonPostRecordWithMedia), "notificationReasonPostRecordWithMedia" },
        { int(Role::NotificationReasonPostNotFound), "notificationReasonPostNotFound" },
        { int(Role::NotificationReasonPostLabels), "notificationReasonPostLabels" },
        { int(Role::NotificationReasonPostLocallyDeleted), "notificationReasonPostLocallyDeleted" },
        { int(Role::NotificationTimestamp), "notificationTimestamp" },
        { int(Role::NotificationSecondsAgo), "notificationSecondsAgo" },
        { int(Role::NotificationIsRead), "notificationIsRead" },
        { int(Role::NotificationPostUri), "notificationPostUri" },
        { int(Role::NotificationCid), "notificationCid" },
        { int(Role::NotificationLabels), "notificationLabels" },
        { int(Role::NotificationPostAuthor), "notificationPostAuthor" },
        { int(Role::NotificationPostText), "notificationPostText" },
        { int(Role::NotificationPostPlainText), "notificationPostPlainText" },
        { int(Role::NotificationPostLanguages), "notificationPostLanguages" },
        { int(Role::NotificationPostTimestamp), "notificationPostTimestamp" },
        { int(Role::NotificationPostHasUnknownEmbed), "notificationPostHasUnknownEmbed" },
        { int(Role::NotificationPostUnknownEmbedType), "notificationPostUnknownEmbedType" },
        { int(Role::NotificationPostImages), "notificationPostImages" },
        { int(Role::NotificationPostVideo), "notificationPostVideo" },
        { int(Role::NotificationPostExternal), "notificationPostExternal" },
        { int(Role::NotificationPostRecord), "notificationPostRecord" },
        { int(Role::NotificationPostRecordWithMedia), "notificationPostRecordWithMedia" },
        { int(Role::NotificationPostReplyRootAuthorDid), "notificationPostReplyRootAuthorDid" },
        { int(Role::NotificationPostReplyRootUri), "notificationPostReplyRootUri" },
        { int(Role::NotificationPostReplyRootCid), "notificationPostReplyRootCid" },
        { int(Role::NotificationPostMentionDids), "notificationPostMentionDids" },
        { int(Role::NotificationPostRepostUri), "notificationPostRepostUri" },
        { int(Role::NotificationPostLikeUri), "notificationPostLikeUri" },
        { int(Role::NotificationPostLikeTransient), "notificationPostLikeTransient" },
        { int(Role::NotificationPostThreadMuted), "notificationPostThreadMuted" },
        { int(Role::NotificationPostReplyDisabled), "notificationPostReplyDisabled" },
        { int(Role::NotificationPostEmbeddingDisabled), "notificationPostEmbeddingDisabled" },
        { int(Role::NotificationPostViewerStatePinned), "notificationPostViewerStatePinned" },
        { int(Role::NotificationPostThreadgateUri), "notificationPostThreadgateUri" },
        { int(Role::NotificationPostReplyRestriction), "notificationPostReplyRestriction" },
        { int(Role::NotificationPostReplyRestrictionLists), "notificationPostReplyRestrictionLists" },
        { int(Role::NotificationPostHiddenReplies), "notificationPostHiddenReplies" },
        { int(Role::NotificationPostIsHiddenReply), "notificationPostIsHiddenReply" },
        { int(Role::NotificationPostRepostCount), "notificationPostRepostCount" },
        { int(Role::NotificationPostLikeCount), "notificationPostLikeCount" },
        { int(Role::NotificationPostQuoteCount), "notificationPostQuoteCount" },
        { int(Role::NotificationPostReplyCount), "notificationPostReplyCount" },
        { int(Role::NotificationPostBookmarked), "notificationPostBookmarked" },
        { int(Role::NotificationPostBookmarkTransient), "notificationPostBookmarkTransient" },
        { int(Role::NotificationPostNotFound), "notificationPostNotFound" },
        { int(Role::NotificationPostBlocked), "notificationPostBlocked" },
        { int(Role::NotificationPostLabels), "notificationPostLabels" },
        { int(Role::NotificationPostContentVisibility), "notificationPostContentVisibility" },
        { int(Role::NotificationPostContentWarning), "notificationPostContentWarning" },
        { int(Role::NotificationPostContentLabeler), "notificationPostContentLabeler" },
        { int(Role::NotificationPostMutedReason), "notificationPostMutedReason" },
        { int(Role::NotificationPostIsReply), "notificationPostIsReply" },
        { int(Role::ReplyToAuthor), "replyToAuthor" },
        { int(Role::NotificationInviteCode), "notificationInviteCode" },
        { int(Role::NotificationInviteCodeUsedBy), "notificationInviteCodeUsedBy" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void NotificationListModel::postIndexedSecondsAgoChanged()
{
    changeData({ int(Role::NotificationSecondsAgo) });
}

void NotificationListModel::likeCountChanged()
{
    changeData({ int(Role::NotificationPostLikeCount) });
}

void NotificationListModel::likeUriChanged()
{
    changeData({ int(Role::NotificationPostLikeUri) });
}

void NotificationListModel::likeTransientChanged()
{
    changeData({ int(Role::NotificationPostLikeTransient) });
}

void NotificationListModel::replyCountChanged()
{
    changeData({ int(Role::NotificationPostReplyCount) });
}

void NotificationListModel::repostCountChanged()
{
    changeData({ int(Role::NotificationPostRepostCount) });
}

void NotificationListModel::quoteCountChanged()
{
    changeData({ int(Role::NotificationPostQuoteCount) });
}

void NotificationListModel::repostUriChanged()
{
    changeData({ int(Role::NotificationPostRepostUri) });
}

void NotificationListModel::threadgateUriChanged()
{
    changeData({ int(Role::NotificationPostThreadgateUri) });
}

void NotificationListModel::replyRestrictionChanged()
{
    changeData({ int(Role::NotificationPostReplyRestriction) });
}

void NotificationListModel::replyRestrictionListsChanged()
{
    changeData({ int(Role::NotificationPostReplyRestrictionLists) });
}

void NotificationListModel::hiddenRepliesChanged()
{
    changeData({ int(Role::NotificationPostHiddenReplies), int(Role::NotificationPostIsHiddenReply) });
}

void NotificationListModel::threadMutedChanged()
{
    changeData({ int(Role::NotificationPostThreadMuted) });
}

void NotificationListModel::detachedRecordChanged()
{
    changeData({ int(Role::NotificationPostRecord), int(Role::NotificationPostRecordWithMedia) });
}

void NotificationListModel::reAttachedRecordChanged()
{
    changeData({ int(Role::NotificationPostRecord), int(Role::NotificationPostRecordWithMedia) });
}

void NotificationListModel::viewerStatePinnedChanged()
{
    changeData({ int(Role::NotificationPostViewerStatePinned) });
}

void NotificationListModel::postDeletedChanged()
{
    changeData({ int(Role::NotificationReasonPostLocallyDeleted) });
}

void NotificationListModel::locallyBlockedChanged()
{
    changeData({ int(Role::NotificationPostBlocked) });
}

void NotificationListModel::bookmarkedChanged()
{
    changeData({ int(Role::NotificationPostBookmarked) });
}

void NotificationListModel::bookmarkTransientChanged()
{
    changeData({ int(Role::NotificationPostBookmarkTransient) });
}

void NotificationListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}


BasicProfile NotificationListModel::getContentLabeler(QEnums::ContentVisibility visibility,
                                                      const ContentLabelList& labels,
                                                      int labelIndex) const
{
    if (visibility == QEnums::CONTENT_VISIBILITY_SHOW)
        return {};

    if (labelIndex < 0 || labelIndex >= labels.size())
        return {};

    const QString& labelerDid = labels[labelIndex].getDid();

    if (labelerDid.isEmpty())
        return {};

    const BasicProfile* profile = AuthorCache::instance().get(labelerDid);

    if (profile)
        return *profile;

    QTimer::singleShot(0, this, [labelerDid]{ AuthorCache::instance().putProfile(labelerDid); });
    return {};
}
}
