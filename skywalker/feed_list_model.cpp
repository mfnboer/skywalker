// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "feed_list_model.h"
#include "favorite_feeds.h"

namespace Skywalker {

FeedListModel::FeedListModel(const FavoriteFeeds& favoriteFeeds, QObject* parent) :
    QAbstractListModel(parent),
    mFavoriteFeeds(favoriteFeeds)
{
    connect(&mFavoriteFeeds, &FavoriteFeeds::feedSaved, this, [this]{ feedSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::feedPinned, this, [this]{ feedPinnedChanged(); });
}

int FeedListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mFeeds.size();
}

QVariant FeedListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeeds.size())
        return {};

    const auto& feed = mFeeds[index.row()];
    const auto* change = getLocalChange(feed.getCid());

    switch (Role(role))
    {
    case Role::Feed:
        return QVariant::fromValue(feed);
    case Role::FeedLikeCount:
        return feed.getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::FeedLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : feed.getViewer().getLike();
    case Role::FeedCreator:
    {
        auto creator = feed.getCreator();
        const Profile* profileChange = getProfileChange(creator.getDid());
        return QVariant::fromValue(profileChange ? *profileChange : creator);
    }
    case Role::FeedSaved:
        return mFavoriteFeeds.isSavedFeed(feed.getUri());
    case Role::FeedPinned:
        return mFavoriteFeeds.isPinnedFeed(feed.getUri());
    case Role::EndOfFeed:
        return index.row() == (int)mFeeds.size() - 1 && isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void FeedListModel::clear()
{
    qDebug() << "Clear feeds";

    if (!mFeeds.empty())
    {
        beginRemoveRows({}, 0, mFeeds.size() - 1);
        mFeeds.clear();
        endRemoveRows();
    }

    mCursor.clear();
    clearLocalChanges();
    clearLocalProfileChanges();
}

void FeedListModel::addFeeds(ATProto::AppBskyFeed::GeneratorViewList feeds, const QString& cursor)
{
    qDebug() << "Add feeds:" << feeds.size() << "cursor:" << cursor;
    mCursor = cursor;

    if (feeds.empty())
    {
        qDebug() << "No new feeds";

        if (!mFeeds.empty())
            emit dataChanged(createIndex(mFeeds.size() - 1, 0), createIndex(mFeeds.size() - 1, 0), { (int)Role::EndOfFeed });

        return;
    }

    const size_t newRowCount = mFeeds.size() + feeds.size();

    beginInsertRows({}, mFeeds.size(), newRowCount - 1);

    for (auto& view : feeds)
    {
        qDebug() << view->mDisplayName;
        ATProto::AppBskyFeed::GeneratorView::SharedPtr sharedRaw(view.release());
        mFeeds.emplace_back(sharedRaw);
    }

    endInsertRows();
    qDebug() << "New feeds size:" << mFeeds.size();
}

void FeedListModel::addFeeds(const GeneratorViewList& feeds)
{
    qDebug() << "Add feeds:" << feeds.size();
    if (feeds.empty())
    {
        qDebug() << "No new feeds";

        if (!mFeeds.empty())
            emit dataChanged(createIndex(mFeeds.size() - 1, 0), createIndex(mFeeds.size() - 1, 0), { (int)Role::EndOfFeed });

        return;
    }

    const size_t newRowCount = mFeeds.size() + feeds.size();

    beginInsertRows({}, mFeeds.size(), newRowCount - 1);
    mFeeds.insert(mFeeds.end(), feeds.begin(), feeds.end());
    endInsertRows();

    qDebug() << "New feeds size:" << mFeeds.size();
}

QHash<int, QByteArray> FeedListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Feed), "feed" },
        { int(Role::FeedLikeCount), "feedLikeCount" },
        { int(Role::FeedLikeUri), "feedLikeUri" },
        { int(Role::FeedCreator), "feedCreator" },
        { int(Role::FeedSaved), "feedSaved" },
        { int(Role::FeedPinned), "feedPinned" },
        { int(Role::EndOfFeed), "endOfFeed" }
    };

    return roles;
}

void FeedListModel::likeCountChanged()
{
    changeData({ int(Role::FeedLikeCount) });
}

void FeedListModel::likeUriChanged()
{
    changeData({ int(Role::FeedLikeUri) });
}

void FeedListModel::profileChanged()
{
    changeData({ int(Role::FeedCreator) });
}

void FeedListModel::feedSavedChanged()
{
    changeData({ int(Role::FeedSaved) });
}

void FeedListModel::feedPinnedChanged()
{
    changeData({ int(Role::FeedPinned) });
}

void FeedListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mFeeds.size() - 1, 0), roles);
}

}
