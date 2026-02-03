// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "filtered_post_base_model.h"
#include "list_store.h"

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

void FilteredPostBaseModel::resetRowFillPosts()
{
    mNumRowFillPosts = 0;
}

void FilteredPostBaseModel::addRowFillPosts()
{
    // For reverse feed order we fill the feed with place holder
    // posts to make the feed size a multiple of the row size to
    // make reverse tile views work. Without the fillers, the
    // posts may shift horizontal position each time a new page
    // of posts get added.

    if (!isReverseFeed())
        return;

    const int remainder = mFeed.size() % mRowSize;

    if (remainder == 0)
        return;

    mNumRowFillPosts = mRowSize - remainder;
    beginInsertRowsPhysical(mFeed.size(), mFeed.size() + mNumRowFillPosts - 1);

    for (int i = 0; i < mNumRowFillPosts; ++i)
        mFeed.push_back(Post::createNotFound());

    endInsertRows();

    qDebug() << getFeedName() << "Add row fill posts:" << mNumRowFillPosts;
}

void FilteredPostBaseModel::removeRowFillPosts()
{
    if (!isReverseFeed())
        return;

    if (mNumRowFillPosts == 0)
        return;

    if (mNumRowFillPosts > (int)mFeed.size())
    {
        qWarning() << getFeedName() << "Fill post:" << mNumRowFillPosts << "feed size:" << mFeed.size();
        mNumRowFillPosts = mFeed.size();
    }

    beginRemoveRowsPhysical(mFeed.size() - mNumRowFillPosts, mFeed.size() - 1);

    for (int i = 0; i < mNumRowFillPosts; ++i)
        mFeed.pop_back();

    endRemoveRows();

    qDebug() << getFeedName() << "Removes row fill posts:" << mNumRowFillPosts;
    mNumRowFillPosts = 0;
}

}
