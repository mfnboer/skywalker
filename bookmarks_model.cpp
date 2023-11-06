// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "bookmarks_model.h"

namespace Skywalker {

BookmarksModel::BookmarksModel(const QString& userDid, const IProfileStore& following,
                               const ContentFilter& contentFilter, QObject* parent) :
    AbstractPostFeedModel(userDid, following, contentFilter, parent)
{
}

void BookmarksModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    clearFeed();
    mRawPosts.clear();
    endRemoveRows();

    qDebug() << "All bookmarks removed";
    // TODO: cache
}

void BookmarksModel::addBookmarks(const std::vector<QString> postUris, ATProto::Client& bsky)
{
    Q_ASSERT(postUris.size() <= MAX_PAGE_SIZE);

    if (postUris.empty())
        return;

    if (mInProgress)
    {
        qDebug() << "Already adding bookmarks";
        return;
    }

    setInProgress(true);

    bsky.getPosts(postUris,
        [this, presence=getPresence()](auto postViewList)
        {
            if (!presence)
                return;

            setInProgress(false);
            beginInsertRows({}, mFeed.size(), mFeed.size() + postViewList.size() - 1);

            for (auto& postView : postViewList)
            {
                Post post(postView.get(), -1);
                mFeed.push_back(post);
                mRawPosts.push_back(std::move(postView));
            }

            endInsertRows();
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

void BookmarksModel::setInProgress(bool inProgress)
{
    if (inProgress != mInProgress)
    {
        mInProgress = inProgress;
        emit inProgressChanged();
    }
}

}
