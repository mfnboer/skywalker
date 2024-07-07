// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "search_post_feed_model.h"

namespace Skywalker {

SearchPostFeedModel::SearchPostFeedModel(const QString& userDid, const IProfileStore& following,
                                         const IProfileStore& mutedReposts,
                                         const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                         const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                         HashtagIndex& hashtags,
                                         QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, focusHashtags, hashtags, parent)
{
}

void SearchPostFeedModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        mRawFeed.clear();
        endRemoveRows();
    }

    mCursorNextPage.clear();
    qDebug() << "All posts removed";
}

int SearchPostFeedModel::setFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    if (!mFeed.empty())
        clear();

    return addFeed(std::forward<ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr>(feed));
}

int SearchPostFeedModel::addFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mPosts.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr>(feed));

    mCursorNextPage = feed->mCursor.value_or("");

    if (mCursorNextPage.isEmpty())
        setEndOfFeed(true);

    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered from page";

        if (mCursorNextPage.isEmpty() && !mFeed.empty())
        {
            mFeed.back().setEndOfFeed(true);
            const auto index = createIndex(mFeed.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });
        }

        return 0;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    mFeed.insert(mFeed.end(), page->mFeed.begin(), page->mFeed.end());
    mRawFeed.push_back(std::move(feed));

    if (mCursorNextPage.isEmpty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
    return page->mFeed.size();
}

void SearchPostFeedModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
}

SearchPostFeedModel::Page::Ptr SearchPostFeedModel::createPage(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed)
{
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mPosts.size(); ++i)
    {
        const auto& feedEntry = feed->mPosts[i];

        if (feedEntry->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry.get(), i);

            if (mustHideContent(post))
                continue;

            preprocess(post);
            page->addPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mRecordType);
            page->addPost(Post::createNotSupported(feedEntry->mRawRecordType));
        }
    }

    qDebug() << "Created page:" << page->mFeed.size() << "posts";
    return page;
}

}
