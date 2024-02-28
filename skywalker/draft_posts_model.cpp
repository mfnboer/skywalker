// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"
#include "draft_posts.h"

namespace Skywalker {

DraftPostsModel::DraftPostsModel(const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const IContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 const IMutedWords& mutedWords, QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, parent)
{
}

int DraftPostsModel::getMaxDrafts() const
{
    return DraftPosts::MAX_DRAFTS;
}

void DraftPostsModel::clear()
{
    qDebug() << "Clear feed";

    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        mRawFeed.clear();
        endRemoveRows();
    }
}

void DraftPostsModel::setFeed(ATProto::AppBskyFeed::PostFeed feed)
{
    qDebug() << "Set feed:" << feed.size();

    if (!mFeed.empty())
        clear();

    mRawFeed = std::move(feed);

    if (mRawFeed.empty())
        return;

    beginInsertRows({}, 0, mRawFeed.size() - 1);

    for (int i = 0; i < (int)mRawFeed.size(); ++i)
    {
        Post post(mRawFeed[i].get(), i);
        mFeed.push_back(post);
    }

    if (!mFeed.empty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();
}

void DraftPostsModel::deleteDraft(int index)
{
    qDebug() << "Delete draft:" << index;

    if (index < 0 || index >= (int)mFeed.size())
        return;

    const bool endOfFeed = getPost(index).isEndOfFeed();

    beginRemoveRows({}, index, index);
    deletePost(index);
    mRawFeed.erase(mRawFeed.begin() + index);
    endRemoveRows();

    if (endOfFeed && !mFeed.empty())
    {
        mFeed.back().setEndOfFeed(true);
        changeData({ int(Role::EndOfFeed) });
    }
}

}
