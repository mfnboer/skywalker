// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "bookmarks_model.h"

namespace Skywalker {

BookmarksModel::BookmarksModel(const QString& userDid, const IProfileStore& following,
                               const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                               QObject* parent) :
    AbstractPostFeedModel(userDid, following, contentFilter, bookmarks, parent)
{
}

void BookmarksModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    qDebug() << "All bookmarks removed";
}

void BookmarksModel::addBookmarks(const std::vector<QString>& postUris, ATProto::Client& bsky)
{
    Q_ASSERT(postUris.size() <= MAX_PAGE_SIZE);

    if (postUris.empty())
        return;

    if (mInProgress)
    {
        qDebug() << "Already adding bookmarks";
        return;
    }

    const std::vector<QString> nonResolvedUris = mPostCache.getNonCachedUris(postUris);

    if (nonResolvedUris.empty())
    {
        addPosts(postUris);
        return;
    }

    setInProgress(true);

    bsky.getPosts(nonResolvedUris,
        [this, presence=getPresence(), postUris](auto postViewList)
        {
            if (!presence)
                return;

            setInProgress(false);
            Q_ASSERT(!postViewList.empty());

            if (postViewList.empty())
            {
                qWarning() << "Did not get bookmarked posts!";
                return;
            }

            for (auto& postView : postViewList)
            {
                Post post(postView.get(), -1);
                ATProto::AppBskyFeed::PostView::SharedPtr sharedRaw(postView.release());
                mPostCache.put(sharedRaw, post);
            }

            addPosts(postUris);
        },
        [this, presence=getPresence()](const QString& error)
        {
            if (!presence)
                return;

            setInProgress(false);
            qWarning() << "Failed to get posts:" << error;
            emit failure(error);
        });

    qDebug() << "Bookmarks:" << mFeed.size();
}

void BookmarksModel::addPosts(const std::vector<QString>& postUris)
{
    beginInsertRows({}, mFeed.size(), mFeed.size() + postUris.size() - 1);

    for (const auto& uri : postUris)
    {
        const Post* post = mPostCache.get(uri);
        Q_ASSERT(post);

        if (post)
            mFeed.push_back(*post);
    }

    if (mFeed.size() == mBookmarks.size())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();
}

void BookmarksModel::setInProgress(bool inProgress)
{
    if (inProgress != mInProgress)
    {
        mInProgress = inProgress;
        emit inProgressChanged();
    }
}

}
