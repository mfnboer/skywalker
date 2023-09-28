// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include "post_record.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_notification.h>

namespace Skywalker {

class PostCache;

class Notification
{
public:
    using Reason = ATProto::AppBskyNotification::NotificationReason;

    explicit Notification(const ATProto::AppBskyNotification::Notification* notification);

    QString getUri() const;
    QString getCid() const;
    Reason getReason() const;
    QString getReasonSubjectUri() const;
    BasicProfile getAuthor() const;
    const QList<BasicProfile>& getOtherAuthors() const { return mOtherAuthors; }
    PostRecord getPostRecord() const;
    Post getReasonPost(const PostCache&) const;
    Post getNotificationPost(const PostCache&) const;
    bool isRead() const;
    QDateTime getTimestamp() const;
    bool isEndOfList() const { return mEndOfList; }
    void setEndOfList(bool endOfFeed) { mEndOfList = endOfFeed; }

    // Get the URI of the post to be displayed, e.g. the uri or subjectReasonUri
    QString getPostUri() const;

    void addOtherAuthor(const BasicProfile& author);

private:
    Post getPost(const PostCache& cache, const QString& uri) const;

    const ATProto::AppBskyNotification::Notification* mNotification = nullptr;
    QList<BasicProfile> mOtherAuthors;
    bool mEndOfList = false;
};

}
