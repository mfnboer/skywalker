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

    void setPostThread(ATProto::AppBskyFeed::PostThread::Ptr&& thread);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::deque<Post> mFeed;
        ATProto::AppBskyFeed::PostThread::Ptr mRawThread;

        void addPost(const Post& post, QEnums::PostType postType);
        void prependPost(const Post& post, QEnums::PostType postType);
        void addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply, QEnums::PostType postType);
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::PostThread::Ptr&& thread);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    ATProto::AppBskyFeed::PostThread::Ptr mRawThread;
};

}
