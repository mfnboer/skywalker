// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "feed_list_model.h"

namespace Skywalker {

FeedListModel::FeedListModel(QObject* parent) :
    QAbstractListModel(parent)
{
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

    switch (Role(role))
    {
    case Role::Feed:
        return QVariant::fromValue(feed);
    case Role::FeedCreator:
        return QVariant::fromValue(feed.getCreator());
    case Role::EndOfeed:
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
        mRawFeeds.clear();
        endRemoveRows();
    }

    mCursor.clear();
}

void FeedListModel::addFeeds(ATProto::AppBskyFeed::GeneratorViewList feeds, const QString& cursor)
{
    qDebug() << "Add feeds:" << feeds.size() << "cursor:" << cursor;
    mCursor = cursor;

    if (feeds.empty())
    {
        qDebug() << "No new feeds";

        if (!mFeeds.empty())
            emit dataChanged(createIndex(mFeeds.size() - 1, 0), createIndex(mFeeds.size() - 1, 0), { (int)Role::EndOfeed });

        return;
    }

    const size_t newRowCount = mFeeds.size() + feeds.size();

    beginInsertRows({}, mFeeds.size(), newRowCount - 1);

    for (const auto& view : feeds)
    {
        qDebug() << view->mDisplayName;
        mFeeds.emplace_back(view.get());
    }

    endInsertRows();

    mRawFeeds.push_back(std::move(feeds));
    qDebug() << "New feeds size:" << mFeeds.size();
}

QHash<int, QByteArray> FeedListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Feed), "feed" },
        { int(Role::FeedCreator), "feedCreator" },
        { int(Role::EndOfeed), "endOfFeed" }
    };

    return roles;
}

}
