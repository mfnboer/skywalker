// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_feed_model.h"
#include "list_store.h"

namespace Skywalker {

AuthorFeedModel::AuthorFeedModel(const DetailedProfile& author, const QString& userDid,
                                 const ContentFilter& contentFilter,
                                 const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, ListStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mAuthor(author),
    mPinnedPostUri(author.getPinnedPostUri())
{
}

QString AuthorFeedModel::getFeedName() const
{
    return QString("%1 feed").arg(mAuthor.getName());
}

void AuthorFeedModel::clear()
{
    if (!mList.empty())
    {
        beginRemoveRows({}, 0, mList.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    qDebug() << "All posts removed";
}

int AuthorFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    if (!mList.empty())
        clear();

    return addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));
}

int AuthorFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::SharedPtr>(feed));

    mCursor = feed->mCursor.value_or("");

    if (mCursor.isEmpty())
        setEndOfFeed(true);

    if (page->mFeed.empty())
    {
        qDebug() << "All posts have been filtered from page";

        if (mCursor.isEmpty() && !mList.empty())
        {
            mList.back().setEndOfFeed(true);
            const auto index = createIndex(mList.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });
        }

        return 0;
    }

    const size_t newRowCount = mList.size() + page->mFeed.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), page->mFeed.begin(), page->mFeed.end());

    if (mCursor.isEmpty())
        mList.back().setEndOfFeed(true);

    endInsertRows();

    qDebug() << "New feed size:" << mList.size();
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

        if (ATProto::holdsNonNull<ATProto::AppBskyFeed::Record::Post::SharedPtr>(feedEntry->mPost->mRecord))
        {
            Post post(feedEntry);

            if (!mustShow(post))
                continue;

            mContentFilterStats.reportChecked(post);

            if (auto reason = mustHideContent(post); reason.first != QEnums::HIDE_REASON_NONE)
            {
                mContentFilterStats.report(post, reason.first, reason.second);
                continue;
            }

            if (mPinnedPostUri == post.getUri())
                post.setPinned(true);

            if (post.isReply() && !post.isRepost() && mustShowReplyContext())
            {
                auto replyRef = post.getViewPostReplyRef();

                if (replyRef && !replyRef->mParent.isBlocked())
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
        else if (ATProto::holdsNonNull<ATProto::UnknownVariant::SharedPtr>(feedEntry->mPost->mRecord))
        {
            const auto unknownRecord = std::get<ATProto::UnknownVariant::SharedPtr>(feedEntry->mPost->mRecord);
            qWarning() << "Unsupported post record type:" << unknownRecord->mType;
            page->addPost(Post::createNotSupported(unknownRecord->mType));
        }
        else
        {
            qWarning() << "Unexpected record type";
            Q_ASSERT(false);
            page->addPost(Post::createNotSupported("UnknownRecord"));
        }
    }

    qDebug() << "Created page:" << page->mFeed.size() << "posts";
    return page;
}

bool AuthorFeedModel::mustShow(const Post& post) const
{
    // A pinned post may be a reply. We show a pinned posts with normal posts.
    switch (mFilter)
    {
    case QEnums::AUTHOR_FEED_FILTER_NONE:
        return true;
    case QEnums::AUTHOR_FEED_FILTER_POSTS:
        return !post.isReply() || post.isRepost() || post.isPinned();
    case QEnums::AUTHOR_FEED_FILTER_REPLIES:
        return post.isReply() && !post.isRepost();
    case QEnums::AUTHOR_FEED_FILTER_MEDIA:
        return (!post.getImages().isEmpty() || post.getVideoView()) && !post.isRepost();
    case QEnums::AUTHOR_FEED_FILTER_VIDEO:
        return post.getVideoView() && !post.isRepost();
    case QEnums::AUTHOR_FEED_FILTER_REPOSTS:
        return true;
    }

    qWarning() << "Unknown filter:" << int(mFilter);
    return false;
}

bool AuthorFeedModel::mustShowReplyContext() const
{
    switch (mFilter)
    {
    case QEnums::AUTHOR_FEED_FILTER_NONE:
    case QEnums::AUTHOR_FEED_FILTER_REPLIES:
        return true;
    default:
        return false;
    }
}

void AuthorFeedModel::getFeed(IFeedPager* pager)
{
    Q_ASSERT(mModelId > -1);
    pager->getAuthorFeed(mModelId);
}

void AuthorFeedModel::getFeedNextPage(IFeedPager* pager)
{
    Q_ASSERT(mModelId > -1);
    pager->getAuthorFeedNextPage(mModelId);
}

}
