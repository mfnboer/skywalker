// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "filtered_search_post_feed_model.h"
#include "search_post_feed_model.h"
#include <ranges>

namespace Skywalker {

FilteredSearchPostFeedModel::FilteredSearchPostFeedModel(IPostFilter::Ptr postFilter,
                                             SearchPostFeedModel* underlyingModel,
                                             const QString& userDid,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    FilteredPostBaseModel(std::move(postFilter), userDid, mutedReposts,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mUnderlyingModel(underlyingModel)
{
}

QVariant FilteredSearchPostFeedModel::getUnderlyingModel()
{
    return QVariant::fromValue(mUnderlyingModel);
}

void FilteredSearchPostFeedModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    setNumPostsChecked(0);
    setCheckedTillTimestamp(QDateTime::currentDateTimeUtc());
    qDebug() << "All posts removed";
}

void FilteredSearchPostFeedModel::setPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    clear();
    addPosts(posts, numPosts);
}

void FilteredSearchPostFeedModel::addPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    qDebug() << "Add posts:" << getFeedName() << "posts:" << numPosts;
    auto page = createPage(posts, 0, numPosts);

    if (page->mFeed.empty())
        setNumPostsChecked(mNumPostsChecked + numPosts);
    else
        setNumPostsChecked(numPosts - page->mFeed.size());

    addPage(std::move(page));

    for (const auto& post : posts | std::ranges::views::reverse)
    {
        if (!post.isPlaceHolder())
        {
            setCheckedTillTimestamp(post.getTimelineTimestamp());
            break;
        }
    }
}

void FilteredSearchPostFeedModel::Page::addPost(const Post* post)
{
    mFeed.push_back(post);
}

FilteredSearchPostFeedModel::Page::Ptr FilteredSearchPostFeedModel::createPage(const TimelineFeed& posts, int startIndex, size_t numPosts)
{
    Q_ASSERT(startIndex + numPosts <= posts.size());
    auto page = std::make_unique<Page>();

    for (int i = startIndex; i < startIndex + (int)numPosts; ++i)
    {
        const auto& post = posts[i];

        if (mPostFilter->match(post))
            page->addPost(&post);
    }

    return page;
}

void FilteredSearchPostFeedModel::addPage(Page::Ptr page)
{
    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered:" << getFeedName();
        return;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    auto posts = page->mFeed | std::views::transform([](const Post* p){ return *p; });
    mFeed.insert(mFeed.end(), posts.begin(), posts.end());
    endInsertRows();

    qDebug() << "Added filtered posts:" << page->mFeed.size() << getFeedName() << mFeed.size();
}

void FilteredSearchPostFeedModel::getFeed(IFeedPager* pager)
{
    if (mUnderlyingModel)
        mUnderlyingModel->getFeed(pager);
    else
        qWarning() << "No underlying model:" << getFeedName();
}

void FilteredSearchPostFeedModel::getFeedNextPage(IFeedPager* pager)
{
    if (mUnderlyingModel)
        mUnderlyingModel->getFeedNextPage(pager);
    else
        qWarning() << "No underlying model:" << getFeedName();
}

}
