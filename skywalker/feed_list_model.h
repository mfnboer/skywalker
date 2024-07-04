// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include "local_feed_model_changes.h"
#include "local_profile_changes.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class FavoriteFeeds;

class FeedListModel : public QAbstractListModel,
                      public LocalFeedModelChanges,
                      public LocalProfileChanges
{
    Q_OBJECT
public:
    enum class Role {
        Feed = Qt::UserRole + 1,
        FeedCreator,
        FeedLikeCount,
        FeedLikeUri,
        FeedSaved,
        FeedPinned,
        EndOfFeed
    };

    using Ptr = std::unique_ptr<FeedListModel>;

    explicit FeedListModel(const FavoriteFeeds& favoriteFeeds, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    void addFeeds(ATProto::AppBskyFeed::GeneratorViewList feeds, const QString& cursor);
    Q_INVOKABLE void addFeeds(const GeneratorViewList& feeds);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }

protected:
    QHash<int, QByteArray> roleNames() const override;

    // LocalFeedModelChanges
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;

private:
    void feedSavedChanged();
    void feedPinnedChanged();
    void changeData(const QList<int>& roles);

    using FeedList = std::deque<GeneratorView>;
    FeedList mFeeds;
    QString mCursor;
    const FavoriteFeeds& mFavoriteFeeds;
};

}
