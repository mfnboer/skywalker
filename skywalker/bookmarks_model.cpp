// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "bookmarks_model.h"
#include "list_store.h"

namespace Skywalker {

BookmarksModel::BookmarksModel(const QString& userDid,
                               const IProfileStore& mutedReposts,
                               const ContentFilter& contentFilter,
                               const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                               HashtagIndex& hashtags,
                               QObject* parent) :
    AbstractPostFeedModel(userDid, mutedReposts, ListStore::NULL_STORE,
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
        std::visit([this, subject=bookmark->mSubject](auto&& post){ addPost(post, subject); }, bookmark->mItem);

    endInsertRows();
    qDebug() << "New bookmarks count:" << mFeed.size();

    if (isEndOfFeed() && !mFeed.empty())
    {
        mFeed.back().setEndOfFeed(true);
        const auto index = createIndex(mFeed.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfFeed) });
    }
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::PostView::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr&)
{
    mFeed.push_back(Post(post));
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::NotFoundPost::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr& subject)
{
    Post p = Post::createNotFound(post->mUri, subject->mCid);
    p.setBookmarked(true);
    mFeed.push_back(p);
}

void BookmarksModel::addPost(const ATProto::AppBskyFeed::BlockedPost::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr& subject)
{
    Post p = Post::createBlocked(post->mUri, subject->mCid);
    p.setBookmarked(true);
    mFeed.push_back(p);
}

}
