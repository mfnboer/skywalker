// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "bookmarks_model.h"
#include "author_cache.h"
#include "definitions.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

BookmarksModel::BookmarksModel(const QString& userDid, const IProfileStore& following,
                               const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                               const MutedWords& mutedWords, QObject* parent) :
    AbstractPostFeedModel(userDid, following, contentFilter, bookmarks, mutedWords, parent)
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
        [this, presence=getPresence(), &bsky, postUris](auto postViewList)
        {
            if (!presence)
                return;

            for (auto& postView : postViewList)
            {
                Post post(postView.get(), -1);
                ATProto::AppBskyFeed::PostView::SharedPtr sharedRaw(postView.release());
                mPostCache.put(sharedRaw, post);
            }

            getAuthorsDeletedPosts(postUris, bsky);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg)
        {
            if (!presence)
                return;

            setInProgress(false);
            qWarning() << "Failed to get posts:" << error << " - " << msg;
            emit failure(msg);
        });

    qDebug() << "Bookmarks:" << mFeed.size();
}

void BookmarksModel::addPosts(const std::vector<QString>& postUris)
{
    beginInsertRows({}, mFeed.size(), mFeed.size() + postUris.size() - 1);

    for (const auto& uri : postUris)
    {
        const Post* post = mPostCache.get(uri);

        if (post)
        {
            mFeed.push_back(*post);
        }
        else
        {
            qWarning() << "Bookmarked post not found:" << uri;
            Post deletedPost(getDeletedPost(uri), -1);
            deletedPost.setBookmarkNotFound(true);
            mFeed.push_back(deletedPost);
        }
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

void BookmarksModel::getAuthorsDeletedPosts(const std::vector<QString>& postUris, ATProto::Client& bsky)
{
    std::unordered_set<QString> unknownAuthors;

    for (const auto& uri : postUris)
    {
        if (mPostCache.contains(uri))
            continue;

        ATProto::ATUri atUri(uri);

        if (!atUri.isValid())
        {
            qWarning() << "Invalid at-uri:" << uri;
            continue;
        }

        const auto& did = atUri.getAuthority();

        if (!AuthorCache::instance().contains(did))
            unknownAuthors.insert(did);
    }

    if (unknownAuthors.empty())
    {
        setInProgress(false);
        addPosts(postUris);
        return;
    }

    const std::vector<QString> authorDidList(unknownAuthors.begin(), unknownAuthors.end());

    bsky.getProfiles(authorDidList,
        [this, presence=getPresence(), postUris](auto profilesViewDetailedList)
        {
            if (!presence)
                return;

            setInProgress(false);

            for (auto& profileViewDetailed : profilesViewDetailedList)
            {
                BasicProfile profile(profileViewDetailed.get());
                AuthorCache::instance().put(profile);
            }

            addPosts(postUris);
        },
        [this, presence=getPresence(), postUris](const QString& error, const QString& msg)
        {
            if (!presence)
                return;

            setInProgress(false);
            qWarning() << "Failed to get authors of deleted posts:" << error << " - " << msg;
            addPosts(postUris);
        });
}

ATProto::AppBskyFeed::PostView* BookmarksModel::getDeletedPost(const QString& atUri)
{
    const auto it = mDeletedPosts.find(atUri);

    if (it != mDeletedPosts.end())
        return it->second.get();

    ATProto::ATUri uri(atUri);
    auto& deletedPost = mDeletedPosts[atUri];

    deletedPost = std::make_unique<ATProto::AppBskyFeed::PostView>();
    deletedPost->mUri = atUri;
    deletedPost->mAuthor = std::make_unique<ATProto::AppBskyActor::ProfileViewBasic>();
    deletedPost->mAuthor->mDid = uri.getAuthority();

    const auto* author = AuthorCache::instance().get(uri.getAuthority());

    if (author)
    {
        deletedPost->mAuthor->mHandle = author->getHandle();

        auto displayName = author->getDisplayName();
        if (!displayName.isEmpty())
            deletedPost->mAuthor->mDisplayName = displayName;

        auto avatar = author->getAvatarUrl();
        if (!avatar.isEmpty())
            deletedPost->mAuthor->mAvatar = avatar;
    }
    else
    {
        deletedPost->mAuthor->mHandle = INVALID_HANDLE;
    }

    deletedPost->mIndexedAt = QDateTime::currentDateTime();
    deletedPost->mRecordType = ATProto::RecordType::APP_BSKY_FEED_POST;
    auto postRecord = std::make_unique<ATProto::AppBskyFeed::Record::Post>();
    postRecord->mText = tr("POST NOT FOUND");
    postRecord->mCreatedAt = deletedPost->mIndexedAt;
    deletedPost->mRecord = std::move(postRecord);

    return deletedPost.get();
}

}
