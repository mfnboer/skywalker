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

    explicit PostThreadModel(const QString& userDid, const IProfileStore& following, QObject* parent = nullptr);

    // Returns index of the entry post
    int setPostThread(ATProto::AppBskyFeed::PostThread::Ptr&& thread);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::deque<Post> mFeed;
        ATProto::AppBskyFeed::PostThread::Ptr mRawThread;
        int mEntryPostIndex = 0;

        Post& addPost(const Post& post);
        Post& prependPost(const Post& post);
        void addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply, bool directReply, bool firstDirectReply);
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::PostThread::Ptr&& thread);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    ATProto::AppBskyFeed::PostThread::Ptr mRawThread;
};

}
