// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"
#include "post_feed_model.h"

namespace Skywalker {

int Post::sNextGapId = 1;

Post Post::createGapPlaceHolder(const QString& gapCursor)
{
    Post post;
    post.mGapCursor = gapCursor;
    post.mGapId = sNextGapId++;
    return post;
}

Post Post::createNotFound()
{
    Post post;
    post.mNotFound = true;
    return post;
}

Post Post::createBlocked()
{
    Post post;
    post.mBlocked = true;
    return post;
}

Post Post::createNotSupported(const QString& unsupportedType)
{
    Post post;
    post.mNotSupported = true;
    post.mUnsupportedType = unsupportedType;
    return post;
}

Post Post::createPost(const ATProto::AppBskyFeed::ThreadElement& threadElement)
{
    switch (threadElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST:
    {
        const auto threadPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(threadElement.mPost).get();
        Q_ASSERT(threadPost);
        Q_ASSERT(threadPost->mPost);
        return Post(threadPost->mPost.get(), -1);
    }
    case ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST:
        return Post::createNotFound();
    case ATProto::AppBskyFeed::PostElementType::BLOCKED_POST:
        return Post::createBlocked();
    case ATProto::AppBskyFeed::PostElementType::POST_VIEW:
    case ATProto::AppBskyFeed::PostElementType::UNKNOWN:
        return Post::createNotSupported(threadElement.mUnsupportedType);
    }

    Q_ASSERT(false);
    qWarning() << "Unexpected thread post type:" << int(threadElement.mType);
    return Post::createNotSupported(QString("Unexpected type: %1").arg(int(threadElement.mType)));
}

Post Post::createPost(const ATProto::AppBskyFeed::ReplyElement& replyElement, int rawIndex)
{
    switch (replyElement.mType)
    {
    case ATProto::AppBskyFeed::PostElementType::POST_VIEW:
    {
        const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(replyElement.mPost).get();
        Q_ASSERT(postView);
        return Post(postView, rawIndex);
    }
    case ATProto::AppBskyFeed::PostElementType::NOT_FOUND_POST:
        return Post::createNotFound();
    case ATProto::AppBskyFeed::PostElementType::BLOCKED_POST:
        return Post::createBlocked();
    case ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST:
    case ATProto::AppBskyFeed::PostElementType::UNKNOWN:
        return Post::createNotSupported(replyElement.mUnsupportedType);
    }

    Q_ASSERT(false);
    qWarning() << "Unexpected thread post type:" << int(replyElement.mType);
    return Post::createNotSupported(QString("Unexpected type: %1").arg(int(replyElement.mType)));
}

Post::Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost, int rawIndex) :
    mFeedViewPost(feedViewPost),
    mRawIndex(rawIndex)
{
    Q_ASSERT((feedViewPost && rawIndex >= 0) || (!feedViewPost && rawIndex == -1));

    if (feedViewPost)
        mPost = feedViewPost->mPost.get();
}

Post::Post(const ATProto::AppBskyFeed::PostView* postView, int rawIndex) :
    mPost(postView),
    mRawIndex(rawIndex)
{
    Q_ASSERT(postView);
}

const QString& Post::getCid() const
{
    static const QString NO_STRING;
    return mPost ? mPost->mCid : NO_STRING;
}

const QString& Post::getUri() const
{
    static const QString NO_STRING;
    return mPost ? mPost->mUri : NO_STRING;
}

QString Post::getText() const
{
    static const QString NO_STRING;

    if (!mPost)
        return NO_STRING;

    // TODO: notFoundPost, blockedPost
    if (mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

        if (record->mFacets.empty())
            return record->mText.toHtmlEscaped().replace('\n', "<br>");
        else
            return ATProto::AppBskyRichtext::applyFacets(record->mText, record->mFacets);
    }

    return NO_STRING;
}

BasicProfile Post::getAuthor() const
{
    return mPost ? BasicProfile(mPost->mAuthor.get()) : BasicProfile();
}

QDateTime Post::getIndexedAt() const
{
    if (!mPost)
        return {};

    // NOTE: the createdAt timestamp is not reliable as clients can put in a local timestamp
    // without timezone (seen in feeds)
    return mPost->mIndexedAt;
}

QDateTime Post::getTimelineTimestamp() const
{
    if (!mReplyRefTimestamp.isNull())
        return mReplyRefTimestamp;

    if (mFeedViewPost && mFeedViewPost->mReason)
        return mFeedViewPost->mReason->mIndexedAt;

    return getIndexedAt();
}

bool Post::isRepost() const
{
    return mFeedViewPost && mFeedViewPost->mReason;
}

std::optional<BasicProfile> Post::getRepostedBy() const
{
    if (!isRepost())
        return {};

    return BasicProfile(mFeedViewPost->mReason->mBy.get());
}

bool Post::isReply() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
        return true;

    if (!mPost)
        return false;

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);
    return record->mReply.get();
}


std::optional<PostReplyRef> Post::getViewPostReplyRef() const
{
    if (!mFeedViewPost || !mFeedViewPost->mReply)
        return {};

    const auto& reply = *mFeedViewPost->mReply;
    PostReplyRef replyRef;
    replyRef.mRoot = Post::createPost(*reply.mRoot, mRawIndex);
    replyRef.mParent = Post::createPost(*reply.mParent, mRawIndex);

    // Set the reference timestamp to the timestap of this reply post.
    // They show up together with this reply post.
    replyRef.mRoot.mReplyRefTimestamp = getTimelineTimestamp();
    replyRef.mParent.mReplyRefTimestamp = getTimelineTimestamp();

    return replyRef;
}

std::optional<BasicProfile> Post::getReplyToAuthor() const
{
    if (mReplyToAuthor)
        return mReplyToAuthor;

    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            return BasicProfile(postView->mAuthor.get());
        }
        else
        {
            return {};
        }
    }

    const auto did = getReplyToAuthorDid();
    if (did.isEmpty())
        return {};

    const auto& authorCache = PostFeedModel::getAuthorCache();
    auto* author = authorCache[did];

    if (!author)
        return {};

    const_cast<Post*>(this)->setReplyToAuthor(author->getProfile());
    return mReplyToAuthor;
}

QString Post::getReplyToCid() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            return postView->mCid;
        }
        else
        {
            return {};
        }
    }

    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

    if (!record->mReply)
        return {};

    return record->mReply->mParent->mCid;
}

QString Post::getReplyToAuthorDid() const
{
    if (mFeedViewPost && mFeedViewPost->mReply)
    {
        if (mFeedViewPost->mReply->mParent->mType == ATProto::AppBskyFeed::PostElementType::POST_VIEW)
        {
            const auto postView = std::get<ATProto::AppBskyFeed::PostView::Ptr>(mFeedViewPost->mReply->mParent->mPost).get();
            return postView->mAuthor->mDid;
        }
        else
        {
            return {};
        }
    }

    if (!mPost)
        return {};

    if (mPost->mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return {};

    const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(mPost->mRecord);

    if (!record->mReply)
        return {};

    const auto& uri = record->mReply->mParent->mUri;

    if (!uri.startsWith("at://did:"))
        return {};

    const int end = uri.indexOf('/', 5);
    if (end < 0)
        return {};

    const auto did = uri.sliced(5, end - 5);
    qDebug() << "Extracted did from uri:" << uri << "did:" << did;
    return did;
}

std::vector<ImageView::Ptr> Post::getImages() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(mPost->mEmbed->mEmbed);
    std::vector<ImageView::Ptr> images;

    for (const auto& img : imagesView->mImages)
        images.push_back(std::make_unique<ImageView>(img.get()));

    return images;
}

ExternalView::Ptr Post::getExternalView() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(mPost->mEmbed->mEmbed)->mExternal;
    return std::make_unique<ExternalView>(external.get());
}

RecordView::Ptr Post::getRecordView() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordView::Ptr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordView>(*recordView);
}

RecordWithMediaView::Ptr Post::getRecordWithMediaView() const
{
    if (!mPost)
        return {};

    if (!mPost->mEmbed || mPost->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::Ptr>(mPost->mEmbed->mEmbed);
    return std::make_unique<RecordWithMediaView>(recordView.get());
}

int Post::getReplyCount() const
{
    return mPost ? mPost->mReplyCount : 0;
}

int Post::getRepostCount() const
{
    return mPost ? mPost->mRepostCount : 0;
}

int Post::getLikeCount() const
{
    return mPost ? mPost->mLikeCount : 0;
}

}
