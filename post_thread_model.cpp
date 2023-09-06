// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_thread_model.h"

namespace Skywalker {

PostThreadModel::PostThreadModel(QObject* parent) :
    AbstractPostFeedModel(parent)
{}

void PostThreadModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize)
{
    mFeed.insert(feedInsertIt, page.mFeed.begin(), page.mFeed.begin() + pageSize);
}

int PostThreadModel::setPostThread(ATProto::AppBskyFeed::PostThread::Ptr&& thread)
{
    if (!mFeed.empty())
        clear();

    auto page = createPage(std::move(thread));

    if (page->mFeed.empty())
    {
        qWarning() << "Page has no posts";
        return -1;
    }

    const size_t newRowCount = page->mFeed.size();

    beginInsertRows({}, 0, newRowCount - 1);
    insertPage(mFeed.end(), *page, newRowCount);
    mRawThread = std::move(page->mRawThread);
    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
    return page->mEntryPostIndex;
}

void PostThreadModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    clearFeed();
    mRawThread = nullptr;
    endRemoveRows();
    qDebug() << "All posts removed";
}

Post& PostThreadModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
    mFeed.back().setPostType(QEnums::POST_THREAD);

    const auto& author = post.getPostView()->mAuthor;
    if (author)
    {
        const BasicProfile authorProfile(author->mHandle, author->mDisplayName.value_or(""));
        AbstractPostFeedModel::cacheAuthorProfile(author->mDid, authorProfile);
    }

    return mFeed.back();
}

Post& PostThreadModel::Page::prependPost(const Post& post)
{
    mFeed.push_front(post);
    mFeed.front().setPostType(QEnums::POST_THREAD);

    // TODO: refactor duplicate code
    const auto& author = post.getPostView()->mAuthor;
    if (author)
    {
        const BasicProfile authorProfile(author->mHandle, author->mDisplayName.value_or(""));
        AbstractPostFeedModel::cacheAuthorProfile(author->mDid, authorProfile);
    }

    return mFeed.front();
}

void PostThreadModel::Page::addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply,
                                           bool directReply, bool firstDirectReply)
{
    switch (reply.mType)
    {
    case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
    {
        const auto& post(std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(reply.mPost));
        Q_ASSERT(post);
        Q_ASSERT(post->mPost);
        auto& threadPost = addPost(Post(post->mPost.get(), -1));
        threadPost.addThreadType(QEnums::THREAD_CHILD);

        if (directReply)
        {
            threadPost.addThreadType(QEnums::THREAD_DIRECT_CHILD);

            if (firstDirectReply)
                threadPost.addThreadType(QEnums::THREAD_FIRST_DIRECT_CHILD);
        }
        else
        {
            threadPost.setParentInThread(true);
        }

        if (!post->mReplies.empty())
        {
            Q_ASSERT(post->mReplies[0]);
            addReplyThread(*post->mReplies[0], false, false);
        }
        else
        {
            mFeed.back().addThreadType(QEnums::THREAD_LEAF);
        }

        break;
    }
    default:
        // TODO: notFound, blocked
        break;
    }

}

PostThreadModel::Page::Ptr PostThreadModel::createPage(ATProto::AppBskyFeed::PostThread::Ptr&& thread)
{
    auto page = std::make_unique<Page>();
    page->mRawThread = std::move(thread);

    ATProto::AppBskyFeed::ThreadViewPost* viewPost = nullptr;
    const auto& postThread = page->mRawThread->mThread;

    switch (postThread->mType)
    {
    case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
    {
        viewPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(postThread->mPost).get();
        Q_ASSERT(viewPost);
        Q_ASSERT(viewPost->mPost);
        auto& pagePost = page->addPost(Post(viewPost->mPost.get(), -1));
        pagePost.addThreadType(QEnums::THREAD_ENTRY);

        if (!viewPost->mParent)
            pagePost.addThreadType(QEnums::THREAD_TOP);
        else
            pagePost.setParentInThread(true);

        if (viewPost->mReplies.empty())
            pagePost.addThreadType(QEnums::THREAD_LEAF);

        break;
    }
    default:
        // TODO: notFound, blocked
        break;
    }

    if (viewPost)
    {
        auto parent = viewPost->mParent.get();
        while (parent)
        {
            switch (parent->mType)
            {
            case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
            {
                const auto post = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(parent->mPost).get();
                Q_ASSERT(post);
                Q_ASSERT(viewPost->mPost);
                auto& pagePost = page->prependPost(Post(post->mPost.get(), -1));
                pagePost.addThreadType(QEnums::THREAD_PARENT);
                parent = post->mParent.get();

                if (!parent)
                    pagePost.addThreadType(QEnums::THREAD_TOP);
                else
                    pagePost.setParentInThread(true);

                break;
            }
            default:
                // TODO: notFound, blocked
                parent = nullptr;
                break;
            }
        }

        // The entry post is now at the end of the feed
        page->mEntryPostIndex = page->mFeed.size() - 1;

        bool firstReply = true;
        for (const auto& reply : viewPost->mReplies)
        {
            Q_ASSERT(reply);
            page->addReplyThread(*reply, true, firstReply);
            firstReply = false;
        }
    }

    return page;
}

}
