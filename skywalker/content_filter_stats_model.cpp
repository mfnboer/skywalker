// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats_model.h"
#include "post_feed_model.h"

namespace Skywalker {

ContentFilterStatsModel::ContentFilterStatsModel(QObject* parent) :
    QAbstractItemModel(parent),
    mRootItem(std::make_unique<ContentFilterStatItem>("root", 0))
{
}

ContentFilterStatsModel::ContentFilterStatsModel(const ContentFilterStats& stats, const IContentFilter& contentFilter, QObject* parent) :
    QAbstractItemModel(parent),
    mRootItem(std::make_unique<ContentFilterStatItem>(tr("Total"), 0))
{
    setStats(stats, contentFilter);
}

template<class Key>
static ContentFilterStatItem* addStat(ContentFilterStatItem* parentItem, const Key& key, int stat, QEnums::HideReasonType hideReason)
{
    if (!parentItem)
        return nullptr;

    if (stat > 0)
    {
        auto item = std::make_unique<ContentFilterStatItem>(key, stat, parentItem);
        item->setHideReason(hideReason);
        auto* addedItem = item.get();
        parentItem->addChild(std::move(item));
        return addedItem;
    }

    return nullptr;
}

template<class Key, class StatsList>
static void addStat(ContentFilterStatItem* parentItem, const Key& key, int stat, QEnums::HideReasonType hideReason, const StatsList& statsList)
{
    auto* item = addStat(parentItem, key, stat, hideReason);

    if (!item)
        return;

    for (const auto& [k, s] : statsList)
        addStat(item, k, s, hideReason);
}

void ContentFilterStatsModel::setStats(const ContentFilterStats& stats, const IContentFilter& contentFilter)
{
    mContentFilterStats = stats;

    Q_ASSERT(mRootItem);
    mRootItem->clearChildItems();
    auto* root = mRootItem.get();

    addStat(root, tr("Total filtered posts (out of %1)").arg(stats.checkedPosts()), stats.total(), QEnums::HIDE_REASON_NONE);
    addStat(root, tr("Muted users"), stats.mutedAuthor(), QEnums::HIDE_REASON_MUTED_AUTHOR, stats.authorsMutedAuthor());
    addStat(root, tr("Muted reposts"), stats.repostsFromAuthor(), QEnums::HIDE_REASON_REPOST_FROM_AUTHOR, stats.authorsRepostsFromAuthor());

    addStat(root, tr("Hide from following feed (via list)"), stats.hideFromFollowingFeed(), QEnums::HIDE_REASON_HIDE_FROM_FOLLOWING_FEED, stats.authorsHideFromFollowingFeed());

    auto* labelItem = addStat(root, tr("Label"), stats.label(), QEnums::HIDE_REASON_LABEL);

    if (labelItem)
    {
        for (const auto& [labelerDid, labelStats] : stats.labelMap())
        {
            int labelCount = 0;

            for (const auto& [_, count] : labelStats)
                labelCount += count;

            auto* labelerItem = addStat(labelItem, ContentFilterStatItem::LabelerDid{labelerDid}, labelCount, QEnums::HIDE_REASON_LABEL);

            if (labelerItem)
            {
                for (const auto& [labelId, count] : labelStats)
                {
                    auto* contentGroup = contentFilter.getContentGroup(labelerDid, labelId);
                    const QString label = contentGroup ? contentGroup->getTitleWithSeverity() : labelId;
                    addStat(labelerItem, label, count, QEnums::HIDE_REASON_LABEL);
                }
            }
        }
    }

    addStat(root, tr("Muted words"), stats.mutedWord(), QEnums::HIDE_REASON_MUTED_WORD, stats.entriesMutedWord());
    addStat(root, tr("Hide users you follow"), stats.hideFollowingFromFeed(), QEnums::HIDE_REASON_HIDE_FOLLOWING_FROM_FEED);
    addStat(root, tr("Language"), stats.language(), QEnums::HIDE_REASON_LANGUAGE, stats.entriesLanguage());
    addStat(root, tr("Quotes with blocked post"), stats.quotesBlockedPost(), QEnums::HIDE_REASON_QUOTE_BLOCKED_POST);
    addStat(root, tr("Replies in thread from not-followed users"), stats.repliesThreadUnfollowed(), QEnums::HIDE_REASON_REPLY_THREAD_UNFOLLOWED);
    addStat(root, tr("Replies to not-followed users"), stats.repliesFromUnfollowed(), QEnums::HIDE_REASON_REPLY_TO_UNFOLLOWED);
    addStat(root, tr("Self-reposts"), stats.selfReposts(), QEnums::HIDE_REASON_SELF_REPOST);
    addStat(root, tr("Reposted posts from followed users"), stats.followingReposts(), QEnums::HIDE_REASON_FOLLOWING_REPOST);

    addStat(root, tr("Replies"), stats.replies(), QEnums::HIDE_REASON_REPLY);
    addStat(root, tr("Reposts"), stats.reposts(), QEnums::HIDE_REASON_REPOST);
    addStat(root, tr("Quotes"), stats.quotes(), QEnums::HIDE_REASON_QUOTE);

    addStat(root, tr("Content imcompatible with feed"), stats.contentMode(), QEnums::HIDE_REASON_CONTENT_MODE);
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
    case Role::HideReason:
        return item->hideReason();
    }

    return {};
}

Qt::ItemFlags ContentFilterStatsModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant ContentFilterStatsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != (int)Role::Value)
        return {};

    if (section == 0)
        return tr("Filter reason");

    return tr("Posts");
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

void ContentFilterStatsModel::setFilteredPostFeed(PostFeedModel* model, QEnums::HideReasonType hideReason) const
{
    mContentFilterStats.setFeed(model, hideReason);
}

QHash<int, QByteArray> ContentFilterStatsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::ValueType), "valueType" },
        { int(Role::Value), "value" },
        { int(Role::HideReason), "hideReason" }
    };

    return roles;
}

}
