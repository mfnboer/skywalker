// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "external_view.h"
#include "image_view.h"
#include "profile.h"
#include "record_view.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class Post
{
public:
    explicit Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost);

    QString getText() const;
    BasicProfile getAuthor() const;
    QDateTime getCreatedAt() const;
    std::optional<BasicProfile> getRepostedBy() const;
    std::vector<ImageView::Ptr> getImages() const;
    ExternalView::Ptr getExternalView() const;
    RecordView::Ptr getRecordView() const;

private:
    const ATProto::AppBskyFeed::FeedViewPost* mFeedViewPost;
};

}
