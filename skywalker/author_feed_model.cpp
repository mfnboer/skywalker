// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_feed_model.h"

namespace Skywalker {

AuthorFeedModel::AuthorFeedModel(const DetailedProfile& author, const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, focusHashtags, hashtags, parent),
    mAuthor(author),
    mPinnedPostUri(author.getPinnedPostUri())
{
}

void AuthorFeedModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    mCursorNextPage.clear();
    qDebug() << "All posts removed";
}

int AuthorFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    if (!mFeed.empty())
        clear();

    return addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));
}

int AuthorFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));

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

    if (mCursorNextPage.isEmpty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
    return page->mFeed.size();
}

void AuthorFeedModel::Page::addPost(const Post& post)
{
    mFeed.push_back(post);
}

AuthorFeedModel::Page::Ptr AuthorFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    auto page = std::make_unique<Page>();

    for (size_t i = 0; i < feed->mFeed.size(); ++i)
    {
        const auto& feedEntry = feed->mFeed[i];

        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry);

            if (!mustShow(post))
                continue;

            if (mustHideContent(post))
                continue;

            if (mPinnedPostUri == post.getUri())
                post.setPinned(true);

            if (post.isReply() && !post.isRepost())
            {
                auto replyRef = post.getViewPostReplyRef();

                if (replyRef)
                {
                    page->addPost(replyRef->mParent);
                    post.setPostType(QEnums::POST_LAST_REPLY);
                    post.setParentInThread(true);
                    auto& parentPost = page->mFeed.back();
                    parentPost.setPostType(QEnums::POST_ROOT);
                }
            }

            preprocess(post);
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

bool AuthorFeedModel::mustShow(const Post& post) const
{
    switch (mFilter)
    {
    case QEnums::AUTHOR_FEED_FILTER_NONE:
        return true;
    case QEnums::AUTHOR_FEED_FILTER_POSTS:
        return !post.isReply() || post.isRepost();
    case QEnums::AUTHOR_FEED_FILTER_REPLIES:
        return post.isReply() && !post.isRepost();
    case QEnums::AUTHOR_FEED_FILTER_MEDIA:
        return !post.getImages().isEmpty() && !post.isRepost();
    }

    qWarning() << "Unknown filter:" << int(mFilter);
    return false;
}

}
