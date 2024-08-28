// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "external_view.h"
#include "image_view.h"
#include "language_utils.h"
#include "normalized_word_index.h"
#include "profile.h"
#include "record_view.h"
#include "record_with_media_view.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

struct PostReplyRef;

class Post : public NormalizedWordIndex
{
public:
    // A gap place holder is created to represent a gap in the timeline, i.e.
    // missing posts that have not been retrieved. The gapCursor can be use
    // to fetch those posts.
    static Post createGapPlaceHolder(const QString& gapCursor);
    static Post createNotFound();
    static Post createBlocked();
    static Post createNotSupported(const QString& unsupportedType);
    static Post createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement, const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView);
    static Post createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement);

    explicit Post(const ATProto::AppBskyFeed::FeedViewPost::SharedPtr feedViewPost = nullptr);
    explicit Post(const ATProto::AppBskyFeed::PostView::SharedPtr postView);

    const ATProto::AppBskyFeed::PostView* getPostView() const { return mPost.get(); }
    bool isPlaceHolder() const { return !mPost; }
    bool isGap() const { return !mPost && mGapId > 0; }
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

    QString getText() const override;
    QString getFormattedText(const std::set<QString>& emphasizeHashtags = {}) const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    bool isRepost() const;
    std::optional<BasicProfile> getRepostedBy() const;
    bool isReply() const;
    std::optional<PostReplyRef> getViewPostReplyRef() const;
    std::optional<BasicProfile> getReplyToAuthor() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyToRef() const;
    QString getReplyToCid() const;
    QString getReplyToUri() const;
    QString getReplyToAuthorDid() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyRootRef() const;
    QString getReplyRootCid() const;
    QString getReplyRootUri() const;

    QList<ImageView> getImages() const;
    QList<ImageView> getDraftImages() const;
    ExternalView::Ptr getExternalView() const;
    RecordView::Ptr getRecordView() const;
    RecordWithMediaView::Ptr getRecordWithMediaView() const;
    bool isQuotePost() const;

    int getReplyCount() const;
    int getRepostCount() const;
    int getLikeCount() const;
    int getQuoteCount() const;
    QString getRepostUri() const;
    QString getLikeUri() const;
    bool isThreadMuted() const;
    bool isReplyDisabled() const;
    bool isEmbeddingDisabled() const;
    ATProto::AppBskyFeed::ThreadgateView::SharedPtr getThreadgateView() const;
    QString getThreadgateUri() const;
    static QEnums::ReplyRestriction makeReplyRestriction(bool allowMention, bool allowFollowing, bool allowList, bool allowNobody);
    QEnums::ReplyRestriction getReplyRestriction() const;
    ListViewBasicList getReplyRestrictionLists() const;
    QStringList getHiddenReplies() const;

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

    const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& getLabels() const;
    ContentLabelList getLabelsIncludingAuthorLabels() const;

    const LanguageList& getLanguages() const;
    bool hasLanguage() const;

    std::vector<QString> getHashtags() const override;

    bool isBookmarkNotFound() const { return mBookmarkNotFound; }
    void setBookmarkNotFound(bool notFound) { mBookmarkNotFound = notFound; }

private:
    // null is place holder for more posts (gap)
    ATProto::AppBskyFeed::PostView::SharedPtr mPost;

    // null if the post represents a reply ref.
    ATProto::AppBskyFeed::FeedViewPost::SharedPtr mFeedViewPost;

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

    // Placeholder for a bookmarked post that cannot be found (probably deleted).
    bool mBookmarkNotFound = false;

    LanguageList mLanguages;
    ATProto::AppBskyFeed::ThreadgateView::SharedPtr mThreadgateView;

    static int sNextGapId;
};

struct PostReplyRef
{
    Post mRoot;
    Post mParent;
};

}
