// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "external_view.h"
#include "image_view.h"
#include "profile.h"
#include "record_view.h"
#include "record_with_media_view.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

struct PostReplyRef;

class Post
{
public:
    // A gap place holder is created to represent a gap in the timeline, i.e.
    // missing posts that have not been retrieved. The gapCursor can be use
    // to fetch those posts.
    static Post createGapPlaceHolder(const QString& gapCursor);
    static Post createNotFound();
    static Post createBlocked();
    static Post createNotSupported(const QString& unsupportedType);
    static Post createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement);
    static Post createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement, int rawIndex);

    explicit Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost = nullptr, int rawIndex = -1);
    Post(const ATProto::AppBskyFeed::PostView* postView, int rawIndex);

    const ATProto::AppBskyFeed::PostView* getPostView() const { return mPost; }
    bool isPlaceHolder() const { return !mPost; }
    bool isGap() const { return !mPost && mGapId > 0; }
    int getRawIndex() const { return mRawIndex; }
    bool isEndOfFeed() const { return mEndOfFeed; }
    int getGapId() const { return mGapId; }
    const QString& getGapCursor() const { return mGapCursor; }
    QEnums::PostType getPostType() const { return mPostType; }
    bool isParentInThread() const { return mParentInThread; }

    const QString& getCid() const;
    const QString& getUri() const;

    // The indexedAt of a post or repost
    QDateTime getTimelineTimestamp() const;

    void setReplyRefTimestamp(const QDateTime& timestamp) { mReplyRefTimestamp = timestamp; }

    QString getText() const;
    QString getFormattedText() const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    bool isRepost() const;
    std::optional<BasicProfile> getRepostedBy() const;
    bool isReply() const;
    std::optional<PostReplyRef> getViewPostReplyRef() const;
    std::optional<BasicProfile> getReplyToAuthor() const;
    ATProto::ComATProtoRepo::StrongRef::Ptr getReplyToRef() const;
    QString getReplyToCid() const;
    QString getReplyToUri() const;
    QString getReplyToAuthorDid() const;
    ATProto::ComATProtoRepo::StrongRef::Ptr getReplyRootRef() const;
    QString getReplyRootCid() const;
    QString getReplyRootUri() const;

    QList<ImageView> getImages() const;
    ExternalView::Ptr getExternalView() const;
    RecordView::Ptr getRecordView() const;
    RecordWithMediaView::Ptr getRecordWithMediaView() const;
    bool isQuotePost() const;

    int getReplyCount() const;
    int getRepostCount() const;
    int getLikeCount() const;
    QString getRepostUri() const;
    QString getLikeUri() const;

    void setEndOfFeed(bool end) { mEndOfFeed = end; }
    void setPostType(QEnums::PostType postType) { mPostType = postType; }
    void setParentInThread(bool parentInThread) { mParentInThread = parentInThread; }
    void setReplyToAuthor(const BasicProfile& profile) { mReplyToAuthor = profile; }

    int getThreadType() const { return mThreadType; }
    void addThreadType(QEnums::ThreadPostType threadType) { mThreadType |= threadType; }

    bool isNotFound() const { return mNotFound; }
    bool isBlocked() const { return mBlocked; }
    bool isNotSupported() const { return mNotSupported; }
    const QString& getUnsupportedType() const { return mUnsupportedType; }

    const std::vector<ATProto::ComATProtoLabel::Label::Ptr>& getLabels() const;

private:
    struct HyperLink
    {
        int mStart;
        int mEnd;
        QString mText;
    };

    // null is place holder for more posts (gap)
    const ATProto::AppBskyFeed::PostView* mPost = nullptr;

    // null if the post represents a reply ref.
    const ATProto::AppBskyFeed::FeedViewPost* mFeedViewPost = nullptr;

    // Index in the vector of raw feed view posts (only for timeline)
    int mRawIndex = -1;

    int mGapId = 0;

    // cursor to get more posts to fill the gap
    QString mGapCursor;

    bool mEndOfFeed = false;
    QEnums::PostType mPostType = QEnums::POST_STANDALONE;
    int mThreadType = QEnums::THREAD_NONE;

    // Timestamp to keep reply references in time sequence for the timeline
    QDateTime mReplyRefTimestamp;

    // For posts not having all parent informations, the reply-to-author may
    // inferred from through other posts.
    std::optional<BasicProfile> mReplyToAuthor;
    bool mParentInThread = false;

    bool mBlocked = false;
    bool mNotFound = false;
    bool mNotSupported = false;
    QString mUnsupportedType;

    static int sNextGapId;
};

struct PostReplyRef
{
    Post mRoot;
    Post mParent;
};

}
