// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_feed_model.h"
#include <QQmlEngine>
#include <QQmlListProperty>

using namespace std::chrono_literals;

namespace Skywalker {

PostFeedModel::PostFeedModel(QObject* parent) :
    QAbstractListModel(parent)
{}

void PostFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (feed->mFeed.empty())
    {
        qDebug() << "Trying to set an empty feed";
        return;
    }

    if (mFeed.empty())
    {
        addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        return;
    }

    // TODO: is this check needed? we can just replace, right?
    // Use this position the feed to the last viewed post, and to check
    // if there is a gap between that post and the loaded page!
//    const QString& cidFirstPost = feed->mFeed[0]->mPost->mCid;
//    const QString& cidFirstStoredPost = mFeedPages[0]->mRawFeed[0]->mPost->mCid;

//    if (cidFirstPost == cidFirstStoredPost)
//    {
//        qDebug() << "No new post available";
//        return;
//    }

    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
}

void PostFeedModel::insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
#if 0
    Q_ASSERT(!mFeedPages.empty());
    Q_ASSERT(!mFeedPages[0]->mRawFeed.empty());

    qDebug() << "Insert feed at start";
    const QString& cidFirstStoredPost = mFeedPages[0]->mRawFeed[0]->mPost->mCid;

    int index = 0;
    for (const auto& feedEntry : feed->mFeed)
    {
        const QString& cid = feedEntry->mPost->mCid;
        if (cid == cidFirstStoredPost)
            break;

        ++index;
    }

    if (index < feed->mFeed.size())
    {
        qDebug() << "Page overlap:" << feed->mFeed.size() - index;
        feed->mFeed.erase(feed->mFeed.begin() + index, feed->mFeed.end());
    }
    else
    {
        qDebug() << "No page overlap found";
    }

    if (feed->mFeed.empty())
    {
        qDebug() << "Full overlap";
        return;
    }

    auto page = createPage(std::forward<ATProto::AppBskyFeed::PostFeed>(feed->mFeed));
    // TODO: the cursor indicates the next page after a full page, but we truncated it.

    const size_t newRowCount = mRowCount + page->mFeed.size();

    // TODO: maybe use a deque instead of vector
    beginInsertRows({}, 0, page->mFeed.size() - 1);
    mFeedPages.insert(mFeedPages.begin(), std::move(page));
    endInsertRows();

    mRowCount = newRowCount;
    qDebug() << "New feed size:" << mRowCount;
#endif
}

void PostFeedModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    mFeed.clear();
    endRemoveRows();
    qDebug() << "All posts removed";
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();

    if (feed->mFeed.empty())
        return;

    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);

    mFeed.insert(mFeed.end(), page->mFeed.begin(), page->mFeed.end());

    if (!page->mCursorNextPage.isEmpty())
        mIndexCursorMap[mFeed.size() - 1] = page->mCursorNextPage;

    mIndexRawFeedMap[mFeed.size() - 1] = std::move(page->mRawFeed);

    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
}

QString PostFeedModel::getLastCursor() const
{
    if (mIndexCursorMap.empty())
        return {};

    return mIndexCursorMap.rbegin()->second;
}

int PostFeedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mFeed.size();
}

QVariant PostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mFeed.size())
        return {};

    const auto& post = mFeed[index.row()];

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(post.getAuthor());
    case Role::PostText:
        return post.getText();
    case Role::PostIndexedSecondsAgo:
    {
        const auto duration = QDateTime::currentDateTime() - post.getIndexedAt();
        return qint64(duration / 1000ms);
    }
    case Role::PostImages:
    {
        QList<ImageView> images;
        for (const auto& img : post.getImages())
            images.push_back(*img);

        return QVariant::fromValue(images);
    }
    case Role::PostExternal:
    {
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant();
    }
    case Role::PostRepostedByName:
    {
        const auto& repostedBy = post.getRepostedBy();
        return repostedBy ? repostedBy->getName() : QVariant();
    }
    case Role::PostRecord:
    {
        auto record = post.getRecordView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::PostRecordWithMedia:
    {
        auto record = post.getRecordWithMediaView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    default:
        qDebug() << "Uknown role requested:" << role;
        break;
    }

    return {};
}

QHash<int, QByteArray> PostFeedModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Author), "author" },
        { int(Role::PostText), "postText" },
        { int(Role::PostIndexedSecondsAgo), "postIndexedSecondsAgo" },
        { int(Role::PostRepostedByName), "postRepostedByName" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostExternal), "postExternal"},
        { int(Role::PostRecord), "postRecord"},
        { int(Role::PostRecordWithMedia), "postRecordWithMedia"}
    };

    return roles;
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed) const
{
    auto page = std::make_unique<Page>();
    page->mRawFeed = std::forward<ATProto::AppBskyFeed::PostFeed>(feed->mFeed);

    for (const auto& feedEntry : page->mRawFeed)
    {
        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
            page->mFeed.push_back(Post(feedEntry.get()));
        else
            qWarning() << "Unsupported post record type:" << int(feedEntry->mPost->mRecordType);
    }

    if (feed->mCursor && !feed->mCursor->isEmpty())
        page->mCursorNextPage = *feed->mCursor;

    return page;
}

}
