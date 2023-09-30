// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_feed_model.h"

namespace Skywalker {

AuthorFeedModel::AuthorFeedModel(const QString& author, const QString& userDid, const IProfileStore& following, QObject* parent) :
    AbstractPostFeedModel(userDid, following, parent),
    mAuthor(author)
{
}

void AuthorFeedModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    clearFeed();
    mRawFeed.clear();
    endRemoveRows();

    mCursorNextPage.clear();
    qDebug() << "All posts removed";
}

bool AuthorFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (!mFeed.empty())
        clear();

    return addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
}

bool AuthorFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    mCursorNextPage = feed->mCursor.value_or("");
    if (mCursorNextPage.isEmpty())
        setEndOfFeed(true);

    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered from page";
        return false;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    mFeed.insert(mFeed.end(), page->mFeed.begin(), page->mFeed.end());
    mRawFeed.push_back(std::move(feed));
    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
    return true;
}

void AuthorFeedModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
}

AuthorFeedModel::Page::Ptr AuthorFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mFeed.size(); ++i)
    {
        const auto& feedEntry = feed->mFeed[i];

        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry.get(), i);

            if (post.isReply() && !post.isRepost())
                continue;

            page->addPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mPost->mRecordType);
            page->addPost(Post::createNotSupported(feedEntry->mPost->mRawRecordType));
        }
    }

    qDebug() << "Created page:" << page->mFeed.size() << "posts";
    return page;
}

}
