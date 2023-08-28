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

    if (mFeedPages.empty())
    {
        addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        return;
    }

    // TODO: is this check needed? we can just replace, right?
    // Use this position the feed to the last viewed post, and to check
    // if there is a gap between that post and the loaded page!
    const QString& cidFirstPost = feed->mFeed[0]->mPost->mCid;
    const QString& cidFirstStoredPost = mFeedPages[0]->mRawFeed[0]->mPost->mCid;

    if (cidFirstPost == cidFirstStoredPost)
    {
        qDebug() << "No new post available";
    }

    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
}

void PostFeedModel::clear()
{
    beginRemoveRows({}, 0, mRowCount - 1);
    mFeedPages.clear();
    mRowCount = 0;
    endRemoveRows();
    qDebug() << "All posts removed";
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();

    if (feed->mFeed.empty())
        return;

    auto page = createPage(std::forward<ATProto::AppBskyFeed::PostFeed>(feed->mFeed));

    if (feed->mCursor && !feed->mCursor->isEmpty())
        page->mCursorNextPage = *feed->mCursor;

    const size_t newRowCount = mRowCount + page->mFeed.size();

    beginInsertRows({}, mRowCount, newRowCount - 1);
    mFeedPages.push_back(std::move(page));
    endInsertRows();

    mRowCount = newRowCount;
    qDebug() << "New feed size:" << mRowCount;
}

QString PostFeedModel::getLastCursor() const
{
    const auto& cursor = mFeedPages.back()->mCursorNextPage;
    return cursor.isEmpty() ? QString() : cursor;
}

int PostFeedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mRowCount;
}

QVariant PostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mRowCount)
        return {};

    const auto& post = getPost(index.row());

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(post.getAuthor());
    case Role::PostText:
        return post.getText();
    case Role::PostCreatedSecondsAgo:
    {
        const auto duration = QDateTime::currentDateTime() - post.getCreatedAt();
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
        { int(Role::PostCreatedSecondsAgo), "postCreatedSecondsAgo" },
        { int(Role::PostRepostedByName), "postRepostedByName" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostExternal), "postExternal"},
        { int(Role::PostRecord), "postRecord"},
        { int(Role::PostRecordWithMedia), "postRecordWithMedia"}
    };

    return roles;
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::PostFeed&& feed) const
{
    auto page = std::make_unique<Page>();
    page->mRawFeed = std::forward<ATProto::AppBskyFeed::PostFeed>(feed);

    for (const auto& feedEntry : page->mRawFeed)
    {
        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
            page->mFeed.push_back(Post(feedEntry.get()));
        else
            qWarning() << "Unsupported post record type:" << int(feedEntry->mPost->mRecordType);
    }

    return page;
}

const Post& PostFeedModel::getPost(size_t index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < mRowCount);

    for (const auto& page : mFeedPages)
    {
        if (index < page->mFeed.size())
            return page->mFeed[index];

        index -= page->mFeed.size();
    }

    qWarning() << "Invalid index:" << index << "row count:" << mRowCount;
    throw std::invalid_argument("Invalid post index");
}

}
