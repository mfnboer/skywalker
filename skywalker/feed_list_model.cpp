// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "feed_list_model.h"
#include "favorite_feeds.h"
#include "skywalker.h"

namespace Skywalker {

FeedListModel::FeedListModel(const FavoriteFeeds& favoriteFeeds, Skywalker* skywalker,
                             QObject* parent) :
    BaseType(skywalker->getUserDid(), parent),
    mFavoriteFeeds(favoriteFeeds),
    mUserSettings(*skywalker->getUserSettings())
{
    connect(&mFavoriteFeeds, &FavoriteFeeds::feedSaved, this, [this]{ feedSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::feedPinned, this, [this]{ feedPinnedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::feedUnpinned, this, [this](QString){ feedPinnedChanged(); });
}

QVariant FeedListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& feed = mList[index.row()];
    const auto* change = getLocalChange(feed.getCid());

    switch (Role(role))
    {
    case Role::Feed:
        return QVariant::fromValue(feed);
    case Role::FeedLikeCount:
        return feed.getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::FeedLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : feed.getViewer().getLike();
    case Role::FeedLikeTransient:
        return change ? change->mLikeTransient : false;
    case Role::FeedHideFollowing:
        return mUserSettings.getFeedHideFollowing(mUserDid, feed.getUri());
    case Role::FeedCreator:
    {
        auto creator = feed.getCreator();
        const Profile* profileChange = getProfileChange(creator.getDid());
        return QVariant::fromValue(profileChange ? *profileChange : creator);
    }
    case Role::FeedSync:
        return mUserSettings.mustSyncFeed(mUserDid, feed.getUri());
    case Role::FeedSaved:
        return mFavoriteFeeds.isSavedFeed(feed.getUri());
    case Role::FeedPinned:
        return mFavoriteFeeds.isPinnedFeed(feed.getUri());
    case Role::EndOfFeed:
        return index.row() == (int)mList.size() - 1 && isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void FeedListModel::clear()
{
    qDebug() << "Clear feeds";
    BaseType::clear();
    clearLocalChanges();
    clearLocalProfileChanges();
}

void FeedListModel::addFeeds(ATProto::AppBskyFeed::GeneratorView::List feeds, const QString& cursor)
{
    qDebug() << "Add feeds:" << feeds.size() << "cursor:" << cursor;
    mCursor = cursor;

    if (feeds.empty())
    {
        qDebug() << "No new feeds";

        if (!mList.empty())
            emit dataChanged(createIndex(mList.size() - 1, 0), createIndex(mList.size() - 1, 0), { (int)Role::EndOfFeed });

        return;
    }

    const size_t newRowCount = mList.size() + feeds.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (auto& view : feeds)
    {
        qDebug() << view->mDisplayName;
        mList.emplace_back(view);
    }

    endInsertRows();
    qDebug() << "New feeds size:" << mList.size();
}

void FeedListModel::addFeeds(const GeneratorView::List& feeds)
{
    qDebug() << "Add feeds:" << feeds.size();
    if (feeds.empty())
    {
        qDebug() << "No new feeds";

        if (!mList.empty())
            emit dataChanged(createIndex(mList.size() - 1, 0), createIndex(mList.size() - 1, 0), { (int)Role::EndOfFeed });

        return;
    }

    const size_t newRowCount = mList.size() + feeds.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), feeds.begin(), feeds.end());
    endInsertRows();

    qDebug() << "New feeds size:" << mList.size();
}

void FeedListModel::prependFeed(const GeneratorView& feed)
{
    qDebug() << "Prepend feed:" << feed.getName();

    beginInsertRows({}, 0, 0);
    mList.push_front(feed);
    endInsertRows();

    qDebug() << "New feeds size:" << mList.size();
}

void FeedListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return;
    }

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    endRemoveRows();
}

bool FeedListModel::hasFeed(const QString& feedUri) const
{
    return std::find_if(mList.begin(), mList.end(),
                        [feedUri](const auto& feed){ return feed.getUri() == feedUri; }) != mList.end();
}

void FeedListModel::setFeedError(const QString& error)
{
    if (error != mFeedError)
    {
        mFeedError = error;
        emit feedErrorChanged();
    }
}

QHash<int, QByteArray> FeedListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Feed), "feed" },
        { int(Role::FeedLikeCount), "feedLikeCount" },
        { int(Role::FeedLikeUri), "feedLikeUri" },
        { int(Role::FeedLikeTransient), "feedLikeTransient" },
        { int(Role::FeedHideFollowing), "feedHideFollowing" },
        { int(Role::FeedCreator), "feedCreator" },
        { int(Role::FeedSync), "feedSync" },
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

void FeedListModel::likeTransientChanged()
{
    changeData({ int(Role::FeedLikeTransient) });
}

void FeedListModel::hideFollowingChanged()
{
    changeData({ int(Role::FeedHideFollowing) });
}

void FeedListModel::syncFeedChanged()
{
    changeData({ int(Role::FeedSync) });
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
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
