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

QString Notification::getUri() const
{
    return mNotification ? mNotification->mUri : QString();
}

Notification::Reason Notification::getReason() const
{
    return mNotification ? mNotification->mReason : Reason::UNKNOWN;
}

QString Notification::getReasonSubjectUri() const
{
    return mNotification ? mNotification->mReasonSubject.value_or(QString()) : QString();
}

BasicProfile Notification::getAuthor() const
{
    return mNotification ? BasicProfile(mNotification->mAuthor.get()) : BasicProfile();
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
    return mNotification ? mNotification->mIndexedAt : QDateTime();
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
    case Reason::UNKNOWN:
        return getUri();
        break;
    }

    qWarning() << "Invalid reasone:" << (int)getReason();
    return {};
}

void Notification::addOtherAuthor(const BasicProfile& author)
{
    mOtherAuthors.push_back(author);
}

}
