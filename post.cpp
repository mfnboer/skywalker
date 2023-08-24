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

}
