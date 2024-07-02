// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "message_view.h"
#include "post.h"
#include "post_record.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_notification.h>

namespace Skywalker {

class ContentFilter;
class PostCache;

class Notification
{
public:
    using Reason = QEnums::NotificationReason;

    explicit Notification(const ATProto::AppBskyNotification::Notification* notification);
    Notification(const QString& inviteCode, const BasicProfile& usedBy);
    Notification(const MessageView& messageView, const BasicProfile& messageSender);
    explicit Notification(const BasicProfileList& labelersWithLabels);

    QString getUri() const;
    QString getCid() const;
    Reason getReason() const;
    QString getReasonSubjectUri() const;
    BasicProfile getAuthor() const;
    const BasicProfileList& getOtherAuthors() const { return mOtherAuthors; }
    BasicProfileList getAllAuthors() const;
    PostRecord getPostRecord() const;
    Post getReasonPost(const PostCache&) const;
    Post getNotificationPost(const PostCache&) const;
    bool isRead() const;
    void setIsRead(bool isRead) { mIsRead = isRead; }
    QDateTime getTimestamp() const;    
    bool isEndOfList() const { return mEndOfList; }
    void setEndOfList(bool endOfFeed) { mEndOfList = endOfFeed; }

    const QString& getInviteCode() const { return mInviteCode; }
    const BasicProfile& getInviteCodeUsedBy() const { return mInviteCodeUsedBy; }
    void setInviteCodeUsedBy(const BasicProfile& profile) { mInviteCodeUsedBy = profile; }

    // Get the URI of the post to be displayed, e.g. the uri or subjectReasonUri
    QString getPostUri() const;

    void addOtherAuthor(const BasicProfile& author);
    const MessageView& getDirectMessage() const { return mDirectMessage; }

    bool updateNewLabels(const ContentFilter* contentFilter);

private:
    Post getPost(const PostCache& cache, const QString& uri) const;

    const ATProto::AppBskyNotification::Notification* mNotification = nullptr;
    BasicProfileList mOtherAuthors;
    QString mInviteCode;
    BasicProfile mInviteCodeUsedBy;
    MessageView mDirectMessage;
    BasicProfile mMessageSender;
    BasicProfile mLabelerWithNewLabels;
    bool mIsRead = false;
    bool mEndOfList = false;
};

}
