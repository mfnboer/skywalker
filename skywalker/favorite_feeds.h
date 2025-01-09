// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "feed_list_model.h"
#include "favorite_feed_view.h"
#include "generator_view.h"
#include "list_list_model.h"
#include "list_view.h"
#include "search_feed.h"
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <unordered_set>

namespace Skywalker {

class Skywalker;

// Favorite feeds and lists
class FavoriteFeeds : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool updateSavedFeedsModelInProgress READ getUpdateSavedFeedsModelInProgress NOTIFY updateSavedFeedsModelInProgressChanged FINAL)

public:
    explicit FavoriteFeeds(Skywalker* skywalker, QObject* parent = nullptr);
    ~FavoriteFeeds();

    void clear();
    void reset(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref);
    void set(const SearchFeed::List& searchFeeds);

    // Can also be called for list uri's
    Q_INVOKABLE bool isSavedFeed(const QString& uri) const { return mSavedUris.contains(uri); }
    Q_INVOKABLE bool isPinnedFeed(const QString& uri) const { return mPinnedUris.contains(uri); }
    Q_INVOKABLE bool isPinnedSearch(const QString& name) const { return mPinnedSearches.contains(name); }

    Q_INVOKABLE void addFeed(const GeneratorView& feed);
    Q_INVOKABLE void removeFeed(const GeneratorView& feed);
    Q_INVOKABLE void pinFeed(const GeneratorView& feed, bool pin);

    Q_INVOKABLE void addList(const ListView& list);
    Q_INVOKABLE void removeList(const ListView& list);
    Q_INVOKABLE void pinList(const ListView& list, bool pin);

    Q_INVOKABLE void pinSearch(const SearchFeed& search, bool pin);

    Q_INVOKABLE QList<FavoriteFeedView> getPinnedFeeds() const { return mPinnedFeeds; }
    Q_INVOKABLE FavoriteFeedView getPinnedFeed(const QString& uri) const;
    Q_INVOKABLE FavoriteFeedView getPinnedSearch(const QString& name) const;

    bool getUpdateSavedFeedsModelInProgress() const { return mUpdateSavedFeedsModelInProgress; }
    void setUpdateSavedFeedsModelInProgress(bool inProgress);

    Q_INVOKABLE FeedListModel* getSavedFeedsModel();
    Q_INVOKABLE void removeSavedFeedsModel();

    Q_INVOKABLE ListListModel* getSavedListsModel();
    Q_INVOKABLE void removeSavedListsModel();

    void saveTo(ATProto::UserPreferences& userPreferences) const;
    // TODO: save search feeds to UserSettings

signals:
    void feedSaved();
    void listSaved();
    void feedPinned();
    void listPinned();
    void searchPinned(QString name);
    void searchUnpinned(QString name);
    void updateSavedFeedsModelInProgressChanged();

private:
    void addFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void addFeeds(QList<FavoriteFeedView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void pinFeed(const GeneratorView& feed);
    void unpinFeed(const GeneratorView& feed);
    void pinList(const ListView& list);
    void unpinList(const ListView& list);
    void pinSearch(const SearchFeed& search);
    void unpinSearch(const SearchFeed& search);
    void addSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators);
    void addPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators);
    void addPinnedFeed(const ATProto::AppBskyGraph::ListView::SharedPtr& pinnedList);
    void updateSavedViews();
    void updateSavedGeneratorViews();
    void updateSavedListViews();
    void updateSavedListViews(std::vector<QString> listUris);
    void updatePinnedViews();
    void updatePinnedGeneratorViews();
    void updatePinnedListViews();
    void updatePinnedListViews(std::vector<QString> listUris);
    void updateSavedFeedsModel();
    void updateSavedListsModel();
    std::vector<QString> filterUris(const std::vector<QString> uris, char const* collection) const;

    ATProto::UserPreferences::SavedFeedsPref mSavedFeedsPref;
    std::unordered_set<QString> mSavedUris;
    std::unordered_set<QString> mPinnedUris;
    std::unordered_set<QString> mPinnedSearches;
    QList<GeneratorView> mSavedFeeds; // sorted by name
    QList<ListView> mSavedLists; // sorted by name
    QList<FavoriteFeedView> mPinnedFeeds; // sorted by name
    int mSavedFeedsModelId = -1;
    int mSavedListsModelId = -1;
    bool mUpdateSavedFeedsModelInProgress = false;
    Skywalker* mSkywalker;
};

}
