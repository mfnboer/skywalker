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

void PostThreadModel::setPostThread(ATProto::AppBskyFeed::ThreadViewPost::Ptr&& thread)
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

void PostThreadModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
}

void PostThreadModel::Page::prependPost(const Post& post)
{
    mFeed.push_front(post);
}

void PostThreadModel::Page::addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply)
{
    switch (reply.mType)
    {
    case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
    {
        const auto& post(std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(reply.mPost));
        addPost(Post(post->mPost.get(), -1));

        if (!post->mReplies.empty())
            addReplyThread(*post->mReplies[0]);

        break;
    }
    default:
        // TODO: notFound, blocked
        break;
    }

}

PostThreadModel::Page::Ptr PostThreadModel::createPage(ATProto::AppBskyFeed::ThreadViewPost::Ptr&& thread)
{
    auto page = std::make_unique<Page>();
    page->mRawThread = std::move(thread);

    // TODO: set post type
    Post post(page->mRawThread->mPost.get(), -1);
    page->addPost(post);

    auto parent = page->mRawThread->mParent.get();
    while (parent)
    {
        switch (parent->mType)
        {
        case ATProto::AppBskyFeed::ThreadElementType::THREAD_VIEW_POST:
        {
            const auto& post = std::get<ATProto::AppBskyFeed::ThreadViewPost::Ptr>(parent->mPost);
            page->prependPost(Post(post->mPost.get(), -1));
            break;
        }
        default:
            // TODO: notFound, blocked
            break;
        }
    }

    for (const auto& reply : page->mRawThread->mReplies)
    {
        page->addReplyThread(*reply);
    }

    return page;
}

}
