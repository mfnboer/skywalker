// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"

namespace Skywalker {

int Post::sNextGapId = 1;

Post Post::createGapPlaceHolder(const QString& gapCursor)
{
    Post post;
    post.mGapCursor = gapCursor;
    post.mGapId = sNextGapId++;
    return post;
}

Post::Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost, int rawIndex) :
    mFeedViewPost(feedViewPost),
    mRawIndex(rawIndex)
{
    Q_ASSERT((feedViewPost && rawIndex >= 0) || (!feedViewPost && rawIndex == -1));
}

const QString& Post::getCid() const
{
    static const QString NO_STRING;
    return mFeedViewPost ? mFeedViewPost->mPost->mCid : NO_STRING;
}

QString Post::getText() const
{
    static const QString NO_STRING;

    if (!mFeedViewPost)
        return NO_STRING;

    const auto& post = mFeedViewPost->mPost;
    if (post->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
    {
        const auto& record = std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(post->mRecord);

        if (record->mFacets.empty())
            return record->mText.toHtmlEscaped();
        else
            return ATProto::AppBskyRichtext::applyFacets(record->mText, record->mFacets);
    }

    return NO_STRING;
}

BasicProfile Post::getAuthor() const
{
    return mFeedViewPost ? BasicProfile(mFeedViewPost->mPost->mAuthor.get()) : BasicProfile();
}

QDateTime Post::getIndexedAt() const
{
    if (!mFeedViewPost)
        return {};

    // NOTE: the createdAt timestamp is not reliable as clients can put in a local timestamp
    // without timezone (seen in feeds)
    return mFeedViewPost->mPost->mIndexedAt;
}

QDateTime Post::getTimelineTimestamp() const
{
    if (!mFeedViewPost)
        return {};

    if (mFeedViewPost->mReason)
        return mFeedViewPost->mReason->mIndexedAt;

    return getIndexedAt();
}

std::optional<BasicProfile> Post::getRepostedBy() const
{
    if (!mFeedViewPost)
        return {};

    if (!mFeedViewPost->mReason)
        return {};

    return BasicProfile(mFeedViewPost->mReason->mBy.get());
}

std::vector<ImageView::Ptr> Post::getImages() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(post->mEmbed->mEmbed);
    std::vector<ImageView::Ptr> images;

    for (const auto& img : imagesView->mImages)
        images.push_back(std::make_unique<ImageView>(img.get()));

    return images;
}

ExternalView::Ptr Post::getExternalView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(post->mEmbed->mEmbed)->mExternal;
    return std::make_unique<ExternalView>(external.get());
}

RecordView::Ptr Post::getRecordView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordView::Ptr>(post->mEmbed->mEmbed);
    return std::make_unique<RecordView>(*recordView);
}

RecordWithMediaView::Ptr Post::getRecordWithMediaView() const
{
    if (!mFeedViewPost)
        return {};

    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA_VIEW)
        return {};

    const auto& recordView = std::get<ATProto::AppBskyEmbed::RecordWithMediaView::Ptr>(post->mEmbed->mEmbed);
    return std::make_unique<RecordWithMediaView>(recordView.get());
}

}
