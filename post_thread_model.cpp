// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_thread_model.h"
#include "author_cache.h"

namespace Skywalker {

PostThreadModel::PostThreadModel(const QString& userDid, const IProfileStore& following,
                                 const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, following, contentFilter, bookmarks, parent)
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

static void cacheAuthor(const Post& post)
{
    const auto* postView = post.getPostView();
    if (postView)
    {
        const auto& author = postView->mAuthor;
        if (author)
        {
            const BasicProfile authorProfile(author.get());
            AuthorCache::instance().put(authorProfile);
        }
    }
}

Post& PostThreadModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
    mFeed.back().setPostType(QEnums::POST_THREAD);
    cacheAuthor(post);
    return mFeed.back();
}

Post& PostThreadModel::Page::prependPost(const Post& post)
{
    mFeed.push_front(post);
    mFeed.front().setPostType(QEnums::POST_THREAD);
    cacheAuthor(post);
    return mFeed.front();
}

void PostThreadModel::Page::addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply,
                                           bool directReply, bool firstDirectReply)
{
    auto threadPost = Post::createPost(reply);
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

    addPost(threadPost);

    if (!threadPost.isPlaceHolder())
    {
        const auto& post(std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(reply.mPost));
        if (!post->mReplies.empty())
        {
            Q_ASSERT(post->mReplies[0]);
            addReplyThread(*post->mReplies[0], false, false);
        }
        else
        {
            mFeed.back().addThreadType(QEnums::THREAD_LEAF);
        }
    }
    else
    {
        mFeed.back().addThreadType(QEnums::THREAD_LEAF);
    }
}

PostThreadModel::Page::Ptr PostThreadModel::createPage(ATProto::AppBskyFeed::PostThread::Ptr&& thread)
{
    auto page = std::make_unique<Page>();
    page->mRawThread = std::move(thread);

    ATProto::AppBskyFeed::ThreadViewPost* viewPost = nullptr;
    const auto& postThread = page->mRawThread->mThread;
    Post post = Post::createPost(*postThread);
    post.addThreadType(QEnums::THREAD_ENTRY);

    if (!post.isPlaceHolder())
    {
        viewPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(postThread->mPost).get();

        if (!viewPost->mParent)
            post.addThreadType(QEnums::THREAD_TOP);
        else
            post.setParentInThread(true);

        if (viewPost->mReplies.empty())
            post.addThreadType(QEnums::THREAD_LEAF);
    }
    else
    {
        post.addThreadType(QEnums::THREAD_TOP);
        post.addThreadType(QEnums::THREAD_LEAF);
    }

    page->addPost(post);

    if (viewPost)
    {
        auto parent = viewPost->mParent.get();
        while (parent)
        {
            Post parentPost = Post::createPost(*parent);
            parentPost.addThreadType(QEnums::THREAD_PARENT);

            if (!parentPost.isPlaceHolder())
            {
                const auto threadPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(parent->mPost).get();
                parent = threadPost->mParent.get();

                if (!parent)
                    parentPost.addThreadType(QEnums::THREAD_TOP);
                else
                    parentPost.setParentInThread(true);

            }
            else
            {
                parentPost.addThreadType(QEnums::THREAD_TOP);
                parent = nullptr;
            }

            page->prependPost(parentPost);
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
