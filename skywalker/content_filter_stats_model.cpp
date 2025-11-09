// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats_model.h"

namespace Skywalker {

ContentFilterStatsModel::ContentFilterStatsModel(const ContentFilterStats& stats, QObject* parent) :
    QAbstractItemModel(parent),
    mRootItem(std::make_unique<ContentFilterStatItem>(tr("Total"), 0))
{
    setStats(stats);
}

static ContentFilterStatItem* addStat(ContentFilterStatItem* parentItem, const QString& name, int stat)
{
    qDebug() << "name:" << name << "stat:" << stat;

    if (!parentItem)
    {
        qWarning() << "No parent item for:" << name;
        return nullptr;
    }

    if (stat > 0)
    {
        auto item = std::make_unique<ContentFilterStatItem>(name, stat, parentItem);
        auto* addedItem = item.get();
        parentItem->addChild(std::move(item));
        return addedItem;
    }

    return nullptr;
}

void ContentFilterStatsModel::setStats(const ContentFilterStats& stats)
{
    Q_ASSERT(mRootItem);
    mRootItem->clearChildItems();

    addStat(mRootItem.get(), tr("Muted user"), stats.mutedAuthor());
    auto* item = addStat(mRootItem.get(), tr("Muted reposts"), stats.repostsFromAuthor());

    for (const auto& [author, count] : stats.authorsRepostsFromAuthor())
        addStat(item, author.getHandle(), count);

    addStat(mRootItem.get(), tr("Replies in thread from not-followed user"), stats.repliesFromUnfollowed());
    addStat(mRootItem.get(), tr("Replies to not-followed user"), stats.repliesThreadUnfollowed());
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
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto* item = static_cast<const ContentFilterStatItem*>(index.internalPointer());
    return item->data(index.column());
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

}
