// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "feed_list_model.h"
#include "generator_view.h"
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <unordered_set>

namespace Skywalker {

class Skywalker;

class FavoriteFeeds : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<GeneratorView> pinnedFeeds READ getPinnedFeeds NOTIFY pinnedFeedsChanged FINAL)
    Q_PROPERTY(bool updateSavedFeedsModelInProgress READ getUpdateSavedFeedsModelInProgress NOTIFY updateSavedFeedsModelInProgressChanged FINAL)

public:
    explicit FavoriteFeeds(Skywalker* skywalker, QObject* parent = nullptr);
    ~FavoriteFeeds();

    void clear();
    void reset(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref);

    bool isSavedFeed(const QString& uri) const { return mSavedUris.count(uri); }
    bool isPinnedFeed(const QString& uri) const { return mPinnedUris.count(uri); }

    Q_INVOKABLE void addFeed(const GeneratorView& feed);
    Q_INVOKABLE void removeFeed(const GeneratorView& feed);
    Q_INVOKABLE void pinFeed(const GeneratorView& feed, bool pin);

    const QList<GeneratorView>& getPinnedFeeds() const { return mPinnedFeeds; }

    bool getUpdateSavedFeedsModelInProgress() const { return mUpdateSavedFeedsModelInProgress; }
    void setUpdateSavedFeedsModelInProgress(bool inProgress);

    Q_INVOKABLE FeedListModel* getSavedFeedsModel();
    Q_INVOKABLE void removeSavedFeedsModel();

signals:
    void feedSaved();
    void feedPinned();
    void pinnedFeedsChanged();
    void updateSavedFeedsModelInProgressChanged();

private:
    void setFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void pinFeed(const GeneratorView& feed);
    void unpinFeed(const GeneratorView& feed);
    void setSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators);
    void setPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators);
    void updateSavedViews();
    void updatePinnedViews();
    void updateSavedFeedsModel();

    ATProto::UserPreferences::SavedFeedsPref mSavedFeedsPref;
    std::unordered_set<QString> mSavedUris;
    std::unordered_set<QString> mPinnedUris;
    QList<GeneratorView> mSavedFeeds; // sorted by name
    QList<GeneratorView> mPinnedFeeds; // sorted by name
    int mSavedFeedsModelId = -1;
    bool mUpdateSavedFeedsModelInProgress = false;
    Skywalker* mSkywalker;
};

}
