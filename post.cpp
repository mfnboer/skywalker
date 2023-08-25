// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post.h"

namespace Skywalker {

Post::Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost) :
    mFeedViewPost(feedViewPost)
{
    Q_ASSERT(mFeedViewPost);
}

QString Post::getText() const
{
    const auto& post = mFeedViewPost->mPost;
    if (post->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        return std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(post->mRecord)->mText;

    return {};
}

BasicProfile Post::getAuthor() const
{
    return BasicProfile(mFeedViewPost->mPost->mAuthor.get());
}

QDateTime Post::getCreatedAt() const
{
    const auto& post = mFeedViewPost->mPost;
    if (post->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        return std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(post->mRecord)->mCreatedAt;

    return {};
}

std::vector<ImageView::Ptr> Post::getImages() const
{
    const auto& post = mFeedViewPost->mPost;

    if (!post->mEmbed || post->mEmbed->mType != ATProto::AppBskyEmbed::EmbedType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(post->mEmbed->mEmbed);
    std::vector<ImageView::Ptr> images;

    for (const auto& img : imagesView->mImages)
    {
        auto imgPtr = std::make_unique<ImageView>(img->mThumb, img->mFullSize, img->mFullSize);
        images.push_back(std::move(imgPtr));
    }

    return images;
}

}
