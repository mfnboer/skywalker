// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"

namespace Skywalker {

class PostThreadModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<PostThreadModel>;

    explicit PostThreadModel(QObject* parent = nullptr);

    void setPostThread(ATProto::AppBskyFeed::ThreadViewPost::Ptr&& thread);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::deque<Post> mFeed;
        ATProto::AppBskyFeed::ThreadViewPost::Ptr mRawThread;

        void addPost(const Post& post);
        void prependPost(const Post& post);
        void addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply);
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::ThreadViewPost::Ptr&& thread);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    ATProto::AppBskyFeed::ThreadViewPost::Ptr mRawThread;
};

}
