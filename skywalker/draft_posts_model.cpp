// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "draft_posts_model.h"

namespace Skywalker {

DraftPostsModel::DraftPostsModel(const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 const MutedWords& mutedWords, QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, parent)
{
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

}
