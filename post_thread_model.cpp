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

void PostThreadModel::setPostThread(ATProto::AppBskyFeed::PostThread::Ptr&& thread)
{
    if (!mFeed.empty())
        clear();

    auto page = createPage(std::move(thread));

    if (page->mFeed.empty())
    {
        qWarning() << "Page has no posts";
        return;
    }

    const size_t newRowCount = page->mFeed.size();

    beginInsertRows({}, 0, newRowCount - 1);
    insertPage(mFeed.end(), *page, newRowCount);
    mRawThread = std::move(page->mRawThread);
    // TODO: end of feed?
    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
}

void PostThreadModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    mFeed.clear();
    mRawThread = nullptr;
    clearFeed();
    endRemoveRows();
    qDebug() << "All posts removed";
}

void PostThreadModel::Page::addPost(const Post& post, QEnums::PostType postType)
{
    mFeed.push_back(post);
    mFeed.back().setPostType(postType);
    mFeed.back().setParentInThread(true);
}

void PostThreadModel::Page::prependPost(const Post& post, QEnums::PostType postType)
{
    mFeed.push_front(post);
    mFeed.front().setPostType(postType);
    mFeed.back().setParentInThread(postType != QEnums::POST_THREAD_TOP);
}

void PostThreadModel::Page::addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply,
                                           QEnums::PostType postType)
{
    switch (reply.mType)
    {
    case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
    {
        const auto& post(std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(reply.mPost));
        addPost(Post(post->mPost.get(), -1), postType);

        if (!post->mReplies.empty())
            addReplyThread(*post->mReplies[0], QEnums::POST_THREAD_GRANT_CHILD);
        else
            mFeed.back().setPostType(QEnums::POST_THREAD_LEAF);

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
        page->addPost(Post(viewPost->mPost.get(), -1), QEnums::POST_THREAD_ENTRY);
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
                page->prependPost(Post(post->mPost.get(), -1), QEnums::POST_THREAD_PARENT);
                parent = post->mParent.get();

                if (!parent)
                    page->mFeed.front().setPostType(QEnums::POST_THREAD_TOP);

                break;
            }
            default:
                // TODO: notFound, blocked
                parent = nullptr;
                break;
            }
        }

        bool first = true;
        for (const auto& reply : viewPost->mReplies)
        {
            page->addReplyThread(*reply, first ? QEnums::POST_THREAD_FIRST_CHILD : QEnums::POST_THREAD_CHILD);
            first = false;
        }
    }

    return page;
}

}
