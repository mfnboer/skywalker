// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "notification.h"

namespace Skywalker {

Notification::Notification(const ATProto::AppBskyNotification::Notification* notification) :
    mNotification(notification)
{
    Q_ASSERT(notification);
}

Notification::Reason Notification::getReason() const
{
    return mNotification ? mNotification->mReason : Reason::UNKNOWN;
}

QString Notification::getReasonSubjectUri() const
{
    return mNotification ? *mNotification->mReasonSubject : QString();
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

bool Notification::isRead() const
{
    return mNotification ? mNotification->mIsRead : false;
}

QDateTime Notification::getTimestamp() const
{
    return mNotification ? mNotification->mIndexedAt : QDateTime();
}

}
