// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "generator_view.h"
#include "local_feed_model_changes.h"
#include "local_profile_changes.h"
#include "user_settings.h"

namespace Skywalker {

class FavoriteFeeds;
class Skywalker;

class FeedListModel : public BaseListModel<GeneratorView>,
                      public LocalFeedModelChanges,
                      public LocalProfileChanges
{
    Q_OBJECT
    Q_PROPERTY(QString error READ getFeedError NOTIFY feedErrorChanged FINAL)

public:
    enum class Role {
        Feed = Qt::UserRole + 1,
        FeedCreator,
        FeedLikeCount,
        FeedLikeUri,
        FeedLikeTransient,
        FeedHideFollowing,
        FeedSync,
        FeedSaved,
        FeedPinned,
        EndOfFeed
    };

    using Ptr = std::unique_ptr<FeedListModel>;

    explicit FeedListModel(const FavoriteFeeds& favoriteFeeds, Skywalker* skywalker,
                           QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear() override;
    void addFeeds(ATProto::AppBskyFeed::GeneratorView::List feeds, const QString& cursor);
    Q_INVOKABLE void addFeeds(const GeneratorView::List& feeds);

    Q_INVOKABLE void prependFeed(const GeneratorView& feed);
    Q_INVOKABLE void deleteEntry(int index);

    // Linear search
    Q_INVOKABLE bool hasFeed(const QString& feedUri) const;

    void setFeedError(const QString& error);
    void clearFeedError() { setFeedError({}); }
    const QString& getFeedError() const { return mFeedError; }

signals:
    void feedErrorChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

    // LocalFeedModelChanges
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void likeTransientChanged() override;
    virtual void hideFollowingChanged() override;
    virtual void syncFeedChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;
    virtual void locallyBlockedChanged() override {}

private:
    void feedSavedChanged();
    void feedPinnedChanged();
    void changeData(const QList<int>& roles) override;

    using FeedList = EntityList;
    const FavoriteFeeds& mFavoriteFeeds;
    const UserSettings& mUserSettings;
    QString mFeedError;
};

}
