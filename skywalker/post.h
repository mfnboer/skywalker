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
#include "video_view.h"
#include "web_link.h"
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
    static Post createHiddenPosts();
    static Post createNotFound(const QString uri = "", const QString cid = "");
    static Post createBlocked(const QString uri = "", const QString cid = "");
    static Post createNotSupported(const QString& unsupportedType);
    static Post createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement, const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView);
    static Post createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement);
    static Post fromJson(const QJsonObject& json);
    static void initNextGapId(int gapId) { sNextGapId = gapId; }

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

    void setOverrideCid(const QString& cid) { mOverrideCid = cid; }
    void setOverrideUri(const QString& uri) { mOverrideUri = uri; }

    // The indexedAt of a post or repost
    QDateTime getTimelineTimestamp() const;
    QDateTime getRepostTimestamp() const;

    void setReplyRefTimestamp(const QDateTime& timestamp) { mReplyRefTimestamp = timestamp; }

    void setOverrideText(const QString& text) { mOverrideText = text; }
    void setOverrideFormattedText(const QString& formattedText) { mOverrideFormattedText = formattedText; }

    QString getText() const override;
    QString getFormattedText(const std::set<QString>& emphasizeHashtags = {}, const QString& linkColor = {}) const;

    WebLink::List getDraftEmbeddedLinks() const;
    BasicProfile getAuthor() const;
    QString getAuthorDid() const override { return getAuthor().getDid(); }
    QDateTime getIndexedAt() const;
    void setOverrideIndexedAt(QDateTime dateTime) { mOverrideIndexedAt = dateTime; }
    bool isRepost() const;
    std::optional<BasicProfile> getRepostedBy() const;
    QString getReasonRepostUri() const;
    QString getReasonRepostCid() const;
    bool isReply() const;
    void setOverrideIsReply(bool isReply) { mOverrideIsReply = isReply; }
    std::optional<PostReplyRef> getViewPostReplyRef() const;
    std::optional<BasicProfile> getReplyToAuthor() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyToRef() const;
    QString getReplyToCid() const;
    QString getReplyToUri() const;
    QString getReplyToAuthorDid() const;
    QString getReplyRootAuthorDid() const;
    ATProto::ComATProtoRepo::StrongRef::SharedPtr getReplyRootRef() const;
    QString getReplyRootCid() const;
    QString getReplyRootUri() const;
    QString getFeedContext() const;

    bool hasUnknownEmbed() const;
    QString getUnknownEmbedType() const;
    QList<ImageView> getImages() const override;
    bool hasImages() const;
    QList<ImageView> getDraftImages() const;
    VideoView::Ptr getVideoView() const override;
    bool hasVideo() const;
    VideoView::Ptr getDraftVideoView() const;
    ExternalView::Ptr getExternalView() const override;
    bool hasExternal() const;
    RecordView::Ptr getRecordView() const;
    RecordWithMediaView::Ptr getRecordWithMediaView() const;
    RecordView::SharedPtr getRecordViewFromRecordOrRecordWithMedia() const;
    bool isQuotePost() const;

    int getReplyCount() const;
    void setOverrideReplyCount(int count) { mOverrideReplyCount = count; }
    int getRepostCount() const;
    void setOverrideRepostCount(int count) { mOverrideRepostCount = count; }
    int getLikeCount() const;
    void setOverrideLikeCount(int count) { mOverrideLikeCount = count; }
    int getQuoteCount() const;
    void setOverrideQuoteCount(int count) { mOverrideQuoteCount = count; }
    QString getRepostUri() const;
    void setOverrideRepostUri(const QString& uri) { mOverrideRepostUri = uri; }
    QString getLikeUri() const;
    void setOverrideLikeUri(const QString& uri) { mOverrideLikeUri = uri; }
    void setBookmarked(bool bookmarked) { mIsBookmarked = bookmarked; }
    bool isBookmarked() const;
    void setOverrideBookmarked(bool bookmarked) { mOverrideIsBookmarked = bookmarked; }
    bool isThreadMuted() const;
    void setOverrideThreadMuted(bool muted) { mOverrideThreadMuted = muted; }
    bool isReplyDisabled() const;
    void setOverrideReplyDisabled(bool disabled) { mOverrideReplyDisabled = disabled; }
    bool isEmbeddingDisabled() const;
    void setOverrideEmbeddingDisabled(bool disabled) { mOverrideEmbeddingDisabled = disabled; }
    bool isViewerStatePinned() const;

    ATProto::AppBskyFeed::ThreadgateView::SharedPtr getThreadgateView() const;
    void setThreadgateView(const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgate) { mThreadgateView = threadgate; }
    QString getThreadgateUri() const;
    static QEnums::ReplyRestriction makeReplyRestriction(bool allowMention, bool allowFollower, bool allowFollowing, bool allowList, bool allowNobody);
    QEnums::ReplyRestriction getReplyRestriction() const;
    ListViewBasicList getReplyRestrictionLists() const;
    QStringList getHiddenReplies() const;
    bool isHiddenReply() const;

    void setEndOfFeed(bool end) { mEndOfFeed = end; }
    void setPostType(QEnums::PostType postType) { mPostType = postType; }
    void setParentInThread(bool parentInThread) { mParentInThread = parentInThread; }
    void setReplyToAuthor(const BasicProfile& profile) { mReplyToAuthor = profile; }

    QEnums::FoldedPostType getFoldedPostType() const { return mFoldedPostType; }
    void setFoldedPostType(QEnums::FoldedPostType foldedPostType) { mFoldedPostType = foldedPostType; }

    int getThreadType() const { return mThreadType; }
    void setThreadType(int threadType) { mThreadType = threadType; }
    void addThreadType(QEnums::ThreadPostType threadType) { mThreadType |= threadType; }
    void removeThreadType(QEnums::ThreadPostType threadType) { mThreadType &= ~threadType; }
    int getThreadIndentLevel() const { return mThreadIndentLevel; }
    void setThreadIndentLevel(int indentLevel) { mThreadIndentLevel = indentLevel; }

    bool isHiddenPosts() const { return mHiddenPosts; }
    bool isNotFound() const { return mNotFound; }
    bool isBlocked() const { return mBlocked; }
    bool isNotSupported() const { return mNotSupported; }
    const QString& getUnsupportedType() const { return mUnsupportedType; }

    const std::vector<ATProto::ComATProtoLabel::Label::SharedPtr>& getLabels() const;
    const ContentLabelList& getLabelsIncludingAuthorLabels() const;

    const LanguageList& getLanguages() const;
    bool hasLanguage() const;

    QStringList getMentionDids() const;
    std::vector<QString> getHashtags() const override;
    std::vector<QString> getWebLinks() const override;

    bool isPinned() const { return mPinned; }
    void setPinned(bool pinned) { mPinned = pinned; }

    QEnums::TripleBool isThread() const;
    bool isThreadReply() const;

    QJsonObject toJson() const;

private:
    // null is place holder for more posts (gap)
    ATProto::AppBskyFeed::PostView::SharedPtr mPost;

    // null if the post represents a reply ref.
    ATProto::AppBskyFeed::FeedViewPost::SharedPtr mFeedViewPost;

    QString mUri;
    QString mCid;

    QString mOverrideUri;
    QString mOverrideCid;

    QString mOverrideText;
    QString mOverrideFormattedText;

    QDateTime mOverrideIndexedAt;
    std::optional<bool> mOverrideIsReply;

    int mGapId = 0;

    // cursor to get more posts to fill the gap
    QString mGapCursor;

    bool mEndOfFeed = false;
    QEnums::PostType mPostType = QEnums::POST_STANDALONE;
    QEnums::FoldedPostType mFoldedPostType = QEnums::FOLDED_POST_NONE;
    int mThreadType = QEnums::THREAD_NONE;
    int mThreadIndentLevel = 0;

    // Timestamp to keep reply references in time sequence for the timeline
    QDateTime mReplyRefTimestamp;

    // For posts not having all parent informations, the reply-to-author may
    // inferred from through other posts.
    std::optional<BasicProfile> mReplyToAuthor;
    bool mParentInThread = false;

    bool mHiddenPosts = false; // placeholder for hidden replies in thread view
    bool mBlocked = false;
    bool mNotFound = false;
    bool mNotSupported = false;
    QString mUnsupportedType;
    bool mIsBookmarked = false;

    QString mOverrideRepostUri;
    QString mOverrideLikeUri;
    std::optional<bool> mOverrideIsBookmarked;
    std::optional<bool> mOverrideThreadMuted;
    std::optional<bool> mOverrideEmbeddingDisabled;
    std::optional<bool> mOverrideReplyDisabled;

    std::optional<int> mOverrideReplyCount;
    std::optional<int> mOverrideRepostCount;
    std::optional<int> mOverrideLikeCount;
    std::optional<int> mOverrideQuoteCount;

    LanguageList mLanguages;
    ATProto::AppBskyFeed::ThreadgateView::SharedPtr mThreadgateView;

    bool mPinned = false;
    std::optional<ContentLabelList> mLabelsIncludingAuthorLabels;

    static int sNextGapId;
};

struct PostReplyRef
{
    Post mRoot;
    Post mParent;
};

}
