// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post_record.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_notification.h>

namespace Skywalker {

class Notification
{
public:
    using Reason = ATProto::AppBskyNotification::NotificationReason;

    explicit Notification(const ATProto::AppBskyNotification::Notification* notification);

    Reason getReason() const;
    QString getReasonSubjectUri() const;
    BasicProfile getAuthor() const;
    PostRecord getPostRecord() const;
    bool isRead() const;
    QDateTime getTimestamp() const;
    bool isEndOfList() const { return mEndOfList; }
    void setEndOfList(bool endOfFeed) { mEndOfList = endOfFeed; }

private:
    const ATProto::AppBskyNotification::Notification* mNotification = nullptr;
    bool mEndOfList = false;
};

}
