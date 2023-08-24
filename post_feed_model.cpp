// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_feed_model.h"

namespace Skywalker {

PostFeedModel::PostFeedModel(QObject* parent) :
    QAbstractListModel(parent)
{}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::PostFeed&& feed)
{
    // TODO: proper concatenation
    beginInsertRows({}, mFeed.size(), mFeed.size() + feed.size() - 1);
    mFeed = std::move(feed);
    qDebug() << "NEW FEED:" << mFeed.size();
    endInsertRows();
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

    const auto& feedViewPost = mFeed[index.row()];
    const auto& post = feedViewPost->mPost;
    const auto& author = post->mAuthor;

    switch (Role(role))
    {
    case Role::AuthorName:
        return author->mDisplayName ? author->mDisplayName->trimmed() : author->mHandle;
    case Role::Text:
        if (post->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
            return std::get<ATProto::AppBskyFeed::Record::Post::Ptr>(post->mRecord)->mText;
        break;
    default:
        break;
    }

    return {};
}

QHash<int, QByteArray> PostFeedModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::AuthorName), "authorName" },
        { int(Role::Text), "postText" }
    };

    return roles;
}

}
