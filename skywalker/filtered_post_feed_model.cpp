// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "filtered_post_feed_model.h"
#include "post_feed_model.h"
#include <ranges>

namespace Skywalker {

FilteredPostFeedModel::FilteredPostFeedModel(IPostFilter::Ptr postFilter,
                                             PostFeedModel* underlyingModel,
                                             const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    FilteredPostBaseModel(std::move(postFilter), userDid, following, mutedReposts,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mUnderlyingModel(underlyingModel)
{
}

QVariant FilteredPostFeedModel::getUnderlyingModel()
{
    return QVariant::fromValue(mUnderlyingModel);
}

void FilteredPostFeedModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        mGapIdIndexMap.clear();
        endRemoveRows();
    }

    setNumPostsChecked(0);
    setCheckedTillTimestamp(QDateTime::currentDateTimeUtc());
    qDebug() << "All posts removed";
}

void FilteredPostFeedModel::setPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    clear();
    addPosts(posts, numPosts);
}

void FilteredPostFeedModel::addPosts(const TimelineFeed& posts, size_t numPosts)
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

void FilteredPostFeedModel::prependPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    qDebug() << "Prepend posts:" << getFeedName() << "posts:" << numPosts;
    auto page = createPage(posts, 0, numPosts);
    prependPage(std::move(page));
}

void FilteredPostFeedModel::gapFill(const TimelineFeed& posts, size_t numPosts, int gapId)
{
    qDebug() << "Fill gap:" << getFeedName() << gapId << "posts:" << numPosts;

    if (!mGapIdIndexMap.count(gapId))
    {
        qWarning() << "Gap does not exist:" << gapId;
        return;
    }

    const int gapIndex = mGapIdIndexMap[gapId];
    mGapIdIndexMap.erase(gapId);

    if (gapIndex > (int)mFeed.size() - 1)
    {
        qWarning() << "Gap:" << getFeedName() << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return;
    }

    qDebug() << "Gap id:" << gapId << "index:" << gapIndex << "post.gapId:" << mFeed[gapIndex].getGapId();
    Q_ASSERT(mFeed[gapIndex].getGapId() == gapId);

    // Remove gap place holder
    beginRemoveRows({}, gapIndex, gapIndex);
    mFeed.erase(mFeed.begin() + gapIndex);
    addToIndices(-1, gapIndex);
    endRemoveRows();

    qDebug() << "Removed place holder gap post:" << getFeedName() << gapIndex;
    auto page = createPage(posts, 0, numPosts);

    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered:" << getFeedName();
        return;
    }

    beginInsertRows({}, gapIndex, gapIndex + page->mFeed.size() - 1);
    insertPage(mFeed.begin() + gapIndex, *page);
    endInsertRows();
}

void FilteredPostFeedModel::removeHeadPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    qDebug() << "Remove head posts:" << getFeedName() << "posts:" << numPosts;
    auto page = createPage(posts, 0, numPosts);
    const size_t removeCount = page->mFeed.size();
    qDebug() << "Remove filtered head posts:" << getFeedName() << "num:" << removeCount;
    removePosts(0, removeCount);
}

void FilteredPostFeedModel::removeTailPosts(const TimelineFeed& posts, size_t numPosts)
{
    Q_ASSERT(numPosts <= posts.size());
    qDebug() << "Remove tail posts:" << getFeedName() << "posts:" << numPosts;
    auto page = createPage(posts, posts.size() - numPosts, numPosts);
    const size_t removeCount = page->mFeed.size();
    qDebug() << "Remove filtered tail posts:" << getFeedName() << "num:" << removeCount;
    removePosts(mFeed.size() - removeCount, removeCount);
    setNumPostsChecked(0);

    for (const auto& post : posts)
    {
        if (!post.isPlaceHolder())
        {
            setCheckedTillTimestamp(post.getTimelineTimestamp());
            break;
        }
    }
}

void FilteredPostFeedModel::setEndOfFeed(bool endOfFeed)
{
    AbstractPostFeedModel::setEndOfFeed(endOfFeed);

    if (!mFeed.empty())
    {
        mFeed.back().setEndOfFeed(endOfFeed);
        const auto index = createIndex(mFeed.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfFeed) });
    }
}

void FilteredPostFeedModel::getFeed(IFeedPager* pager)
{
    if (mUnderlyingModel)
        mUnderlyingModel->getFeed(pager);
    else
        qWarning() << "No underlying model:" << getFeedName();
}

void FilteredPostFeedModel::getFeedNextPage(IFeedPager* pager)
{
    if (mUnderlyingModel)
        mUnderlyingModel->getFeedNextPage(pager);
    else
        qWarning() << "No underlying model:" << getFeedName();
}

void FilteredPostFeedModel::Page::addPost(const Post* post)
{
    mFeed.push_back(post);

    if (post->getGapId() != 0)
        mGapIdIndexMap[post->getGapId()] = mFeed.size() - 1;
}

int FilteredPostFeedModel::Page::addThread(const TimelineFeed& posts, int startIndex, size_t numPosts, int matchedPostIndex)
{
    Q_ASSERT(startIndex >= 0);
    Q_ASSERT(numPosts > 0);
    const int endIndex = startIndex + numPosts - 1;
    Q_ASSERT(endIndex < (int)posts.size());
    int startThread = matchedPostIndex;

    for (; startThread >= startIndex; --startThread)
    {
        const auto postType = posts[startThread].getPostType();

        if (postType == QEnums::POST_STANDALONE ||
            (postType == QEnums::POST_LAST_REPLY && startThread < matchedPostIndex))
        {
            ++startThread;
            break;
        }
    }

    if (startThread < startIndex)
        startThread = startIndex;

    int endThread = matchedPostIndex;
    for (; endThread <= endIndex; ++endThread)
    {
        const auto postType = posts[endThread].getPostType();

        if (postType == QEnums::POST_STANDALONE ||
            (postType == QEnums::POST_ROOT && endThread > matchedPostIndex))
        {
            --endThread;
            break;
        }
    }

    if (endThread > endIndex)
        endThread = endIndex;

    for (int j = startThread; j <= endThread; ++j)
        addPost(&posts[j]);

    return endThread;
}

FilteredPostFeedModel::Page::Ptr FilteredPostFeedModel::createPage(const TimelineFeed& posts, int startIndex, size_t numPosts)
{
    Q_ASSERT(startIndex + numPosts <= posts.size());
    auto page = std::make_unique<Page>();

    for (int i = startIndex; i < startIndex + (int)numPosts; ++i)
    {
        const auto& post = posts[i];

        // By copying all gaps from the full time line, we can fill them in when they
        // get filled in the full timeline.
        if (post.isGap())
        {
            page->addPost(&post);
            continue;
        }

        if (!mPostFilter->match(post))
            continue;

        if (post.getPostType() == QEnums::POST_STANDALONE || !mPostFilter->mustAddThread())
        {
            page->addPost(&post);
            continue;
        }

        i = page->addThread(posts, startIndex, numPosts, i);
    }

    return page;
}

void FilteredPostFeedModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page)
{
    if (page.mFeed.empty())
    {
        qDebug() << "Nothing to insert:" << getFeedName();
        return;
    }

    if (feedInsertIt != mFeed.end())
        addToIndices(page.mFeed.size(), feedInsertIt - mFeed.begin());

    auto posts = page.mFeed | std::views::transform([](const Post* p){ return *p; });
    const auto insertIt = mFeed.insert(feedInsertIt, posts.begin(), posts.end());
    const int insertIndex = insertIt - mFeed.begin();

    for (const auto& [gapId, gapIndex] : page.mGapIdIndexMap)
    {
        mGapIdIndexMap[gapId] = insertIndex + gapIndex;
    }
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

    qDebug() << "Added filtered posts:" << page->mFeed.size() << getFeedName() << mFeed.size();
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

    qDebug() << "Prepended filtered posts:" << page->mFeed.size() << getFeedName() << mFeed.size();
}

void FilteredPostFeedModel::removePosts(size_t startIndex, size_t count)
{
    qDebug() << "Remove posts, start:" << startIndex << "count:" << count << "feedSize:" << mFeed.size();
    Q_ASSERT(startIndex + count <= mFeed.size());

    if (count == 0)
    {
        qDebug() << "Nothing to remove:" << getFeedName();
        return;
    }

    const size_t endIndex = startIndex + count - 1;

    beginRemoveRows({}, startIndex, endIndex);

    for (auto it = mGapIdIndexMap.begin(); it != mGapIdIndexMap.end(); )
    {
        if (it->second >= startIndex && it->second <= endIndex)
            it = mGapIdIndexMap.erase(it);
        else
            ++it;
    }

    if (endIndex < mFeed.size() - 1)
        addToIndices(-count, endIndex + 1);

    mFeed.erase(mFeed.begin() + startIndex, mFeed.begin() + startIndex + count);
    endRemoveRows();

    qDebug() << "Removed posts:" << count << getFeedName() << mFeed.size();
}

void FilteredPostFeedModel::addToIndices(int offset, size_t startAtIndex)
{
    for (auto& [gapId, index] : mGapIdIndexMap)
    {
        if (index >= startAtIndex)
            index += offset;
    }
}

}
