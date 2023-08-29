// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "external_view.h"
#include "image_view.h"
#include "profile.h"
#include "record_view.h"
#include "record_with_media_view.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class Post
{
public:
    static Post createPlaceHolder(const QString& gapCursor);

    explicit Post(const ATProto::AppBskyFeed::FeedViewPost* feedViewPost = nullptr);

    bool isPlaceHolder() const { return !mFeedViewPost; }
    bool isEndOfFeed() const { return mEndOfFeed; }
    const QString& getGapCursor() const { return mGapCursor; }

    const QString& getCid() const;

    // The indexedAt of a post or repost
    QDateTime getTimelineTimestamp() const;

    const QString& getText() const;
    BasicProfile getAuthor() const;
    QDateTime getIndexedAt() const;
    std::optional<BasicProfile> getRepostedBy() const;
    std::vector<ImageView::Ptr> getImages() const;
    ExternalView::Ptr getExternalView() const;
    RecordView::Ptr getRecordView() const;
    RecordWithMediaView::Ptr getRecordWithMediaView() const;

    void setGapCursor(const QString& gapCursor) { mGapCursor = gapCursor; }
    void setEndOfFeed(bool end) { mEndOfFeed = end; }

private:
    // NULL is place holder for more posts (gap)
    const ATProto::AppBskyFeed::FeedViewPost* mFeedViewPost = nullptr;
    QString mGapCursor; // cursor to get more posts to fill the gap
    bool mEndOfFeed = false;
};

}
