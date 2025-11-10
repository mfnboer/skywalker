// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats_model.h"

namespace Skywalker {

ContentFilterStatsModel::ContentFilterStatsModel(QObject* parent) :
    QAbstractItemModel(parent),
    mRootItem(std::make_unique<ContentFilterStatItem>(tr("Total"), 0))
{
}

ContentFilterStatsModel::ContentFilterStatsModel(const ContentFilterStats& stats, QObject* parent) :
    QAbstractItemModel(parent),
    mRootItem(std::make_unique<ContentFilterStatItem>(tr("Total"), 0))
{
    setStats(stats);
}

template<class Key>
static ContentFilterStatItem* addStat(ContentFilterStatItem* parentItem, const Key& key, int stat)
{
    if (!parentItem)
        return nullptr;

    if (stat > 0)
    {
        auto item = std::make_unique<ContentFilterStatItem>(key, stat, parentItem);
        auto* addedItem = item.get();
        parentItem->addChild(std::move(item));
        return addedItem;
    }

    return nullptr;
}

template<class Key, class StatsList>
static void addStat(ContentFilterStatItem* parentItem, const Key& key, int stat, const StatsList& statsList)
{
    auto* item = addStat(parentItem, key, stat);

    if (!item)
        return;

    for (const auto& [k, s] : statsList)
        addStat(item, k, s);
}

void ContentFilterStatsModel::setStats(const ContentFilterStats& stats)
{
    Q_ASSERT(mRootItem);
    mRootItem->clearChildItems();
    auto* root = mRootItem.get();

    addStat(root, tr("Muted users"), stats.mutedAuthor());
    addStat(root, tr("Muted reposts"), stats.repostsFromAuthor(), stats.authorsRepostsFromAuthor());

    addStat(root, tr("Hide from following feed (via list)"), stats.hideFromFollowingFeed(), stats.authorsHideFromFollowingFeed());
    addStat(root, tr("Label"), stats.label());
    addStat(root, tr("Hide users you follow"), stats.hideFollowingFromFeed());
    addStat(root, tr("Language"), stats.language());
    addStat(root, tr("Quotes with blocked post"), stats.quotesBlockedPost());
    addStat(root, tr("Replies in thread from not-followed users"), stats.repliesFromUnfollowed());
    addStat(root, tr("Replies to not-followed users"), stats.repliesThreadUnfollowed());
    addStat(root, tr("Self-reposts"), stats.selfReposts());
    addStat(root, tr("Reposted posts from followed users"), stats.followingReposts());

    addStat(root, tr("Replies"), stats.replies());
    addStat(root, tr("Reposts"), stats.reposts());
    addStat(root, tr("Quotes"), stats.quotes());

    addStat(root, tr("Content imcompatible with feed"), stats.contentMode());
}

int ContentFilterStatsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    const auto* parentItem = parent.isValid() ?
            static_cast<const ContentFilterStatItem*>(parent.internalPointer()) :
            mRootItem.get();

    return parentItem->childCount();
}

int ContentFilterStatsModel::columnCount(const QModelIndex& parent) const
{
    const auto* parentItem = parent.isValid() ?
            static_cast<const ContentFilterStatItem*>(parent.internalPointer()) :
            mRootItem.get();

    return parentItem->columnCount();
}

QVariant ContentFilterStatsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    const auto* item = static_cast<const ContentFilterStatItem*>(index.internalPointer());

    if (!item)
        return {};

    switch (Role(role))
    {
    case Role::ValueType:
        return item->valueType(index.column());
    case Role::Value:
        return item->data(index.column());
    }

    return {};
}

Qt::ItemFlags ContentFilterStatsModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant ContentFilterStatsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return orientation == Qt::Horizontal && role == Qt::DisplayRole ?
            mRootItem->data(section) : QVariant{};
}

QModelIndex ContentFilterStatsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    const auto* parentItem = parent.isValid() ?
            static_cast<const ContentFilterStatItem*>(parent.internalPointer()) :
            mRootItem.get();

    if (auto *childItem = parentItem->getChild(row))
        return createIndex(row, column, childItem);

    return {};
}

QModelIndex ContentFilterStatsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    const auto* childItem = static_cast<const ContentFilterStatItem*>(index.internalPointer());
    const auto* parentItem = childItem->getParent();

    if (!parentItem)
        return {};

    return createIndex(parentItem->row(), 0, parentItem);
}

QHash<int, QByteArray> ContentFilterStatsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::ValueType), "valueType" },
        { int(Role::Value), "value" }
    };

    return roles;
}

}
