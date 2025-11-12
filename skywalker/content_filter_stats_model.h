// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include "content_filter_stat_item.h"
#include "content_filter_stats.h"
#include <QAbstractItemModel>
#include <QQmlEngine>

namespace Skywalker {

class PostFeedModel;

class ContentFilterStatsModel : public QAbstractItemModel
{
    Q_MOC_INCLUDE("post_feed_model.h")

    Q_OBJECT
    QML_ELEMENT

public:
    enum class Role {
        ValueType = Qt::UserRole + 1,
        Value,
        KeyList,
        HideReason
    };
    Q_ENUM(Role)

    explicit ContentFilterStatsModel(QObject* parent = nullptr);
    explicit ContentFilterStatsModel(const ContentFilterStats& stats, const IContentFilter& contentFilter, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Q_INVOKABLE void setFilteredPostFeed(PostFeedModel* model, QVariantList keyList) const;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void setStats(const ContentFilterStats& stats, const IContentFilter& contentFilter);

    ContentFilterStats mContentFilterStats;
    ContentFilterStatItem::Ptr mRootItem;
};

}
