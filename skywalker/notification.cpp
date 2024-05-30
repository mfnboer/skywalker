// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification.h"
#include "post_cache.h"

namespace Skywalker {

Notification::Notification(const ATProto::AppBskyNotification::Notification* notification) :
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
        return mNotification->mReason;

    if (!mInviteCode.isEmpty())
        return Reason::INVITE_CODE_USED;

    if (!mDirectMessage.isNull())
        return Reason::DIRECT_MESSAGE;

    return Reason::UNKNOWN;
}

QString Notification::getReasonSubjectUri() const
{
    return mNotification ? mNotification->mReasonSubject.value_or(QString()) : QString();
}

BasicProfile Notification::getAuthor() const
{
    if (mNotification)
        return BasicProfile(mNotification->mAuthor.get());

    if (!mMessageSender.isNull())
        return mMessageSender;

    return {};
}

PostRecord Notification::getPostRecord() const
{
    if (!mNotification)
        return {};

    ATProto::AppBskyFeed::Record::Post* rawRecord = nullptr;

    try {
        rawRecord = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mNotification->mRecord).get();
    } catch (const std::bad_variant_access&) {
        return {};
    }

    return PostRecord(rawRecord);
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
    return mNotification ? mNotification->mIsRead : false;
}

QDateTime Notification::getTimestamp() const
{
    if (mNotification)
        return mNotification->mIndexedAt;

    if (!mDirectMessage.isNull())
        return mDirectMessage.getSentAt();

    return {};
}

QString Notification::getPostUri() const
{
    switch (getReason())
    {
    case Reason::LIKE:
    case Reason::FOLLOW:
    case Reason::REPOST:
        return getReasonSubjectUri();
        break;
    case Reason::REPLY:
    case Reason::MENTION:
    case Reason::QUOTE:
        return getUri();
        break;
    case Reason::INVITE_CODE_USED:
    case Reason::DIRECT_MESSAGE:
    case Reason::UNKNOWN:
        return getUri();
        break;
    }

    qWarning() << "Invalid reason:" << (int)getReason();
    return {};
}

void Notification::addOtherAuthor(const BasicProfile& author)
{
    mOtherAuthors.push_back(author);
}

}
