// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "bookmarks_model.h"

namespace Skywalker {

BookmarksModel::BookmarksModel(const QString& userDid, const IProfileStore& following,
                               const IProfileStore& mutedReposts,
                               const ContentFilter& contentFilter,
                               const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                               HashtagIndex& hashtags,
                               QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, ProfileStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent)
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

    mCursorNextPage.clear();
    qDebug() << "All bookmarks removed";
}

void BookmarksModel::addBookmarks(const ATProto::AppBskyBookmark::GetBookmarksOutput& output)
{
    qDebug() << "Add bookmarks:" << output.mBookmarks.size() << "cursor:" << output.mCursor.value_or("");
    mCursorNextPage = output.mCursor.value_or("");

    // HACK: Bluesky never returns an empty cursor
    if (output.mBookmarks.empty())
        mCursorNextPage = "";

    if (mCursorNextPage.isEmpty())
        setEndOfFeed(true);

    const size_t newRowCount = mFeed.size() + output.mBookmarks.size();
    beginInsertRows({}, mFeed.size(), newRowCount - 1);

    for (const auto& bookmark : output.mBookmarks)
        std::visit([this](auto&& post){ addPost(post); }, bookmark->mItem);

    endInsertRows();
    qDebug() << "New bookmarks count:" << mFeed.size();

    if (isEndOfFeed() && !mFeed.empty())
    {
        mFeed.back().setEndOfFeed(true);
        const auto index = createIndex(mFeed.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfFeed) });
    }
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::PostView::SharedPtr& post)
{
    mFeed.push_back(Post(post));
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::NotFoundPost::SharedPtr& post)
{
    mFeed.push_back(Post::createNotFound(post->mUri));
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::BlockedPost::SharedPtr& post)
{
    mFeed.push_back(Post::createBlocked(post->mUri));
}

}
