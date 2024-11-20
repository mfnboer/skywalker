// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "filtered_post_feed_model.h"
#include <ranges>

namespace Skywalker {

FilteredPostFeedModel::FilteredPostFeedModel(const IPostFilter& postFilter,
                                             const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const Bookmarks& bookmarks,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords,
                  focusHashtags, hashtags, parent),
    mPostFilter(postFilter)
{}

void FilteredPostFeedModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    qDebug() << "All posts removed";
}

void FilteredPostFeedModel::setPosts(const TimelineFeed& posts, const QString& cursor)
{
    clear();
    addPosts(posts, cursor);
}

void FilteredPostFeedModel::addPosts(const TimelineFeed& posts, const QString& cursor)
{
    qDebug() << "Add posts:" << getFeedName() << "posts:" << posts.size() << "cursor:" << cursor;
    auto page = createPage(posts, cursor);
    addPage(std::move(page));
}

void FilteredPostFeedModel::prependPosts(const TimelineFeed& posts, const QString& cursor)
{
    qDebug() << "Prepend posts:" << getFeedName() << "posts:" << posts.size() << "cursor:" << cursor;
    auto page = createPage(posts, cursor);
    prependPage(std::move(page));
}

void FilteredPostFeedModel::Page::addPost(const Post* post)
{
    mFeed.push_back(post);
}

int FilteredPostFeedModel::Page::addThread(const TimelineFeed& posts, int matchedPostIndex)
{
    int startThread = matchedPostIndex;
    for (; startThread >= 0; --startThread)
    {
        if (posts[startThread].getPostType() == QEnums::POST_STANDALONE)
        {
            ++startThread;
            break;
        }
    }

    int endThread = matchedPostIndex;
    for (; endThread < (int)posts.size(); ++endThread)
    {
        if (posts[endThread].getPostType() == QEnums::POST_STANDALONE)
        {
            --endThread;
            break;
        }
    }

    for (int j = startThread; j <= endThread; ++j)
        addPost(&posts[j]);

    return endThread;
}

FilteredPostFeedModel::Page::Ptr FilteredPostFeedModel::createPage(const TimelineFeed& posts, const QString& cursor)
{
    auto page = std::make_unique<Page>();
    page->mCursorNextPage = cursor;

    for (int i = 0; i < (int)posts.size(); ++i)
    {
        const auto& post = posts[i];

        if (!mPostFilter.match(post))
            continue;

        if (post.getPostType() == QEnums::POST_STANDALONE)
        {
            page->addPost(&post);
            continue;
        }

        i = page->addThread(posts, i);
    }

    return page;
}

void FilteredPostFeedModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page)
{
    auto posts = page.mFeed | std::views::transform([](const Post* p){ return *p; });
    mFeed.insert(feedInsertIt, posts.begin(), posts.end());
}

void FilteredPostFeedModel::addPage(Page::Ptr page)
{
    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered:" << getFeedName();
        return;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    insertPage(mFeed.end(), *page);
    endInsertRows();
}

void FilteredPostFeedModel::prependPage(Page::Ptr page)
{
    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered:" << getFeedName();
        return;
    }

    beginInsertRows({}, 0, page->mFeed.size() - 1);
    insertPage(mFeed.begin(), *page);
    endInsertRows();
}

}
