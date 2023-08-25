// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_view.h"
#include "profile.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class Post
{
public:
    explicit Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost);

    QString getText() const;
    BasicProfile getAuthor() const;
    QDateTime getCreatedAt() const;
    std::vector<ImageView::Ptr> getImages() const;

private:
    const ATProto::AppBskyFeed::FeedViewPost* mFeedViewPost;
};

}
