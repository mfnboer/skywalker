// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class PostFeedModel;

class PostFeedReplay
{
public:
    using Ptr = std::unique_ptr<PostFeedReplay>;

    void clear();
    void append(ATProto::AppBskyFeed::OutputFeed::SharedPtr page);
    void prepend(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, bool gapTillNextPage);
    void gapFill(ATProto::AppBskyFeed::OutputFeed::SharedPtr page, const QString& cursor, bool gapTillNextPage);
    void closeGap(const QString& cursor);
    void removeTail(const QString& cursor);
    void removeHead(const QString& cursor);

    void replay(PostFeedModel& model) const;

private:
    struct PageList
    {
        ATProto::AppBskyFeed::OutputFeed::List mPages;
        bool mGapTillNextPage = false;
    };

    PageList* findGap(const QString& cursor);

    std::vector<PageList> mPagePrependSequence;
};

}
