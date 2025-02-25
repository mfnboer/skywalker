// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "generator_view.h"
#include "local_feed_model_changes.h"
#include "local_profile_changes.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class FavoriteFeeds;

class FeedListModel : public QAbstractListModel,
                      public BaseListModel,
                      public LocalFeedModelChanges,
                      public LocalProfileChanges
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)

public:
    enum class Role {
        Feed = Qt::UserRole + 1,
        FeedCreator,
        FeedLikeCount,
        FeedLikeUri,
        FeedLikeTransient,
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

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void getFeedInProgressChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    // LocalFeedModelChanges
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void likeTransientChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;
    virtual void locallyBlockedChanged() override {}

private:
    void feedSavedChanged();
    void feedPinnedChanged();
    void changeData(const QList<int>& roles) override;

    using FeedList = std::deque<GeneratorView>;
    FeedList mFeeds;
    QString mCursor;
    const FavoriteFeeds& mFavoriteFeeds;
    bool mGetFeedInProgress = false;
};

}
