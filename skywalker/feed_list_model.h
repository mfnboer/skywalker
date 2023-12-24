// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "favorite_feeds.h"
#include "generator_view.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class FeedListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Feed = Qt::UserRole + 1,
        FeedCreator,
        FeedSaved,
        FeedPinned,
        EndOfeed
    };

    using Ptr = std::unique_ptr<FeedListModel>;

    explicit FeedListModel(const FavoriteFeeds& favoriteFeeds, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addFeeds(ATProto::AppBskyFeed::GeneratorViewList feeds, const QString& cursor);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    using FeedList = std::deque<GeneratorView>;
    FeedList mFeeds;
    QString mCursor;
    const FavoriteFeeds& mFavoriteFeeds;
};

}
