// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification.h"
#include "content_filter.h"
#include "post_cache.h"

namespace Skywalker {

Notification::Notification(const ATProto::AppBskyNotification::Notification::SharedPtr& notification) :
    mNotification(notification)
{
    Q_ASSERT(notification);
}

Notification::Notification(const QString& inviteCode, const BasicProfile& usedBy) :
    mInviteCode(inviteCode),
    mInviteCodeUsedBy(usedBy)
{
}

Notification::Notification(const MessageView& messageView, const BasicProfile& messageSender) :
    mDirectMessage(messageView),
    mMessageSender(messageSender)
{
}

Notification::Notification(const MessageAndReactionView& messageAndReactionView, const BasicProfile& reactionSender) :
    mDirectMessageAndRection(messageAndReactionView),
    mMessageSender(reactionSender)
{
}

Notification::Notification(const BasicProfile& labeler, const std::unordered_set<QString> newLabelIds) :
    mLabelerWithNewLabels(labeler),
    mNewLabelIds(newLabelIds)
{
}

QString Notification::getUri() const
{
    return mNotification ? mNotification->mUri : QString();
}

QString Notification::getCid() const
{
    return mNotification ? mNotification->mCid : QString();
}

Notification::Reason Notification::getReason() const
{
    if (mNotification)
        return static_cast<Reason>(mNotification->mReason);

    if (!mDirectMessage.isNull())
        return Reason::NOTIFICATION_REASON_DIRECT_MESSAGE;

    if (!mDirectMessageAndRection.isNull())
        return Reason::NOTIFICATION_REASON_DIRECT_MESSAGE_REACTION;

    if (!mLabelerWithNewLabels.isNull())
        return Reason::NOTIFICATION_REASON_NEW_LABELS;

    if (!mInviteCode.isEmpty())
        return Reason::NOTIFICATION_REASON_INVITE_CODE_USED;

    return Reason::NOTIFICATION_REASON_UNKNOWN;
}

QString Notification::getRawReason() const
{
    if (mNotification)
        return mNotification->mRawReason;

    return {};
}

bool Notification::isAggregatable() const
{
    switch (getReason())
    {
    case Notification::Reason::NOTIFICATION_REASON_LIKE:
    case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
    case Notification::Reason::NOTIFICATION_REASON_FOLLOW:
    case Notification::Reason::NOTIFICATION_REASON_REPOST:
    case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
    case Notification::Reason::NOTIFICATION_REASON_VERIFIED:
    case Notification::Reason::NOTIFICATION_REASON_UNVERIFIED:
    case Notification::Reason::NOTIFICATION_REASON_UNKNOWN:
        return true;
    default:
        return false;
    }
}

QString Notification::getReasonSubjectUri() const
{
    switch (getReason())
    {
    case Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
        return getLikeSubjectUri();
    case Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
        return getRepostSubjectUri();
    default:
        return mNotification ? mNotification->mReasonSubject.value_or(QString()) : QString();
    }
}

BasicProfile Notification::getAuthor() const
{
    if (mNotification)
        return BasicProfile(mNotification->mAuthor);

    if (!mMessageSender.isNull())
        return mMessageSender;

    if (!mLabelerWithNewLabels.isNull())
        return mLabelerWithNewLabels;

    return {};
}

BasicProfileList Notification::getAllAuthors() const
{
    const BasicProfile mainAuthor = getAuthor();

    if (mainAuthor.isNull())
        return {};

    BasicProfileList authors;
    authors.reserve(mOtherAuthors.size() + 1);
    authors.push_back(mainAuthor);
    authors.append(mOtherAuthors);

    return authors;
}

PostRecord Notification::getPostRecord() const
{
    if (!mNotification)
        return {};

    ATProto::AppBskyFeed::Record::Post* rawRecord = nullptr;

    try {
        rawRecord = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(mNotification->mRecord).get();
    } catch (const std::bad_variant_access&) {
        return {};
    }

    return PostRecord(rawRecord);
}

QString Notification::getLikeSubjectUri() const
{
    if (!mNotification)
        return {};

    ATProto::AppBskyFeed::Like* rawRecord = nullptr;

    try {
        rawRecord = std::get<ATProto::AppBskyFeed::Like::SharedPtr>(mNotification->mRecord).get();
    } catch (const std::bad_variant_access&) {
        return {};
    }

    if (rawRecord)
        return rawRecord->mSubject->mUri;

    return {};
}

QString Notification::getRepostSubjectUri() const
{
    if (!mNotification)
        return {};

    ATProto::AppBskyFeed::Repost* rawRecord = nullptr;

    try {
        rawRecord = std::get<ATProto::AppBskyFeed::Repost::SharedPtr>(mNotification->mRecord).get();
    } catch (const std::bad_variant_access&) {
        return {};
    }

    if (rawRecord)
        return rawRecord->mSubject->mUri;

    return {};
}

Post Notification::getReasonPost(const PostCache& cache) const
{
    if (!mNotification)
        return Post::createNotFound();

    return getPost(cache, getReasonSubjectUri());
}

Post Notification::getNotificationPost(const PostCache& cache) const
{
    if (!mNotification)
        return Post::createNotFound();

    return getPost(cache, getUri());
}

Post Notification::getPost(const PostCache& cache, const QString& uri) const
{
    if (uri.isEmpty())
        return Post::createNotFound();

    const Post* post = cache.get(uri);
    return post ? *post : Post::createNotFound();
}

bool Notification::isRead() const
{
    return mNotification ? mNotification->mIsRead : mIsRead;
}

QDateTime Notification::getTimestamp() const
{
    if (mNotification)
        return mNotification->mIndexedAt;

    if (!mDirectMessage.isNull())
        return mDirectMessage.getSentAt();

    if (!mLabelerWithNewLabels.isNull())
        return QDateTime::currentDateTime();

    return {};
}

QString Notification::getPostUri() const
{
    switch (getReason())
    {
    case Reason::NOTIFICATION_REASON_LIKE:
    case Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
    case Reason::NOTIFICATION_REASON_FOLLOW:
    case Reason::NOTIFICATION_REASON_REPOST:
    case Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
    case Reason::NOTIFICATION_REASON_STARTERPACK_JOINED:
        return getReasonSubjectUri();
    case Reason::NOTIFICATION_REASON_REPLY:
    case Reason::NOTIFICATION_REASON_MENTION:
    case Reason::NOTIFICATION_REASON_QUOTE:
    case Reason::NOTIFICATION_REASON_SUBSCRIBED_POST:
        return getUri();
    case Reason::NOTIFICATION_REASON_INVITE_CODE_USED:
    case Reason::NOTIFICATION_REASON_DIRECT_MESSAGE:
    case Reason::NOTIFICATION_REASON_DIRECT_MESSAGE_REACTION:
    case Reason::NOTIFICATION_REASON_NEW_LABELS:
    case Reason::NOTIFICATION_REASON_VERIFIED:
    case Reason::NOTIFICATION_REASON_UNVERIFIED:
    case Reason::NOTIFICATION_REASON_UNKNOWN:
        return {};
    }

    qWarning() << "Invalid reason:" << (int)getReason();
    return {};
}

void Notification::addOtherAuthor(const BasicProfile& author)
{
    mOtherAuthors.push_back(author);
}

bool Notification::updateNewLabels(const ContentFilter* contentFilter)
{
    Q_ASSERT(contentFilter);
    Q_ASSERT(!mLabelerWithNewLabels.isNull());

    for (auto it = mOtherAuthors.begin(); it != mOtherAuthors.end(); )
    {
        if (!contentFilter->hasNewLabels(it->getDid()))
            it = mOtherAuthors.erase(it);
        else
            ++it;
    }

    if (!contentFilter->hasNewLabels(mLabelerWithNewLabels.getDid()))
    {
        if (mOtherAuthors.empty())
            return false;

        mLabelerWithNewLabels = mOtherAuthors.back();
        mOtherAuthors.pop_back();
    }

    return true;
}

}
