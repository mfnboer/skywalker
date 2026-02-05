// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "filtered_post_base_model.h"
#include "list_store.h"
#include <ranges>

namespace Skywalker {

FilteredPostBaseModel::FilteredPostBaseModel(IPostFilter::Ptr postFilter,
                                             const QString& userDid,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    AbstractPostFeedModel(userDid, mutedReposts, ListStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mPostFilter(std::move(postFilter))
{
    Q_ASSERT(mPostFilter);
}

void FilteredPostBaseModel::setCheckedTillTimestamp(QDateTime timestamp)
{
    if (timestamp != mCheckedTillTimestamp)
    {
        mCheckedTillTimestamp = timestamp;
        emit checkedTillTimestampChanged();
    }
}

void FilteredPostBaseModel::setNumPostsChecked(int numPostsChecked)
{
    if (numPostsChecked != mNumPostsChecked)
    {
        mNumPostsChecked = numPostsChecked;
        emit numPostsCheckedChanged();
    }
}

void FilteredPostBaseModel::setRowSize(int rowSize)
{
    Q_ASSERT(rowSize > 0);

    if (rowSize <= 0)
    {
        qWarning() << getFeedName() << "Invalid row size:" << rowSize;
        mRowSize = 1;
        return;
    }

    qDebug() << getFeedName() << "Row size:" << rowSize;
    mRowSize = rowSize;
    mRowExcessPosts.reserve(rowSize);
    mAddedExcessPosts.reserve(rowSize);
}

void FilteredPostBaseModel::setEndOfFeed(bool endOfFeed)
{
    AbstractPostFeedModel::setEndOfFeed(endOfFeed);

    for (int i = mFeed.size() - 1; i >= 0; --i)
    {
        auto& post = mFeed[i];

        // There can be place holders at the end for filling up a row in tile view
        if (!post.isPlaceHolder())
        {
            post.setEndOfFeed(true);
            const auto index = createIndex(i, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });
            break;
        }
    }
}

QVariant FilteredPostBaseModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeed.size())
        return {};

    // NOTE: when adding logic here using the index, you must convert to a physical index.

    switch (Role(role))
    {
    case Role::PostType:
        if (!mPostFilter->mustAddThread())
            return QEnums::POST_STANDALONE;
    default:
        break;
    }

    return AbstractPostFeedModel::data(index, role);
}

void FilteredPostBaseModel::resetExcessPosts()
{
    mRowExcessPosts.clear();
    mAddedExcessPosts.clear();
}

void FilteredPostBaseModel::fitToRow(std::vector<const Post*>& pageFeed, bool isLastPage)
{
    // For a reverse feed, we want to add multiples of row size posts, to avoid the tile
    // feed from shifting horizontally each time page gets added.

    if (!isReverseFeed() || mRowSize == 1)
        return;

    mAddedExcessPosts = std::move(mRowExcessPosts);
    mRowExcessPosts.clear();

    for (const auto& post : mAddedExcessPosts | std::ranges::views::reverse)
        pageFeed.insert(pageFeed.begin(), &post);

    const int excess = pageFeed.size() % mRowSize;

    if (excess == 0)
        return;

    if (isLastPage)
    {
        // For that last page we cannot keep posts to be added with the next page.
        // Instead we fill up with empty (not found) post place holders.

        const int fillCount = mRowSize - excess;
        mAddedExcessPosts.push_back(Post::createNotFound());

        for (int i = 0; i < fillCount; ++i)
            pageFeed.push_back(&mAddedExcessPosts.back());
    }
    else
    {
        // Keep posts tot be added with the next page.

        for (const auto* post : pageFeed)
            mRowExcessPosts.push_back(*post);

        pageFeed.erase(pageFeed.end() - excess, pageFeed.end());
    }
}

}
