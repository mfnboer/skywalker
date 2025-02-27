// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "feed_list_model.h"
#include "favorite_feed_view.h"
#include "generator_view.h"
#include "list_list_model.h"
#include "list_view.h"
#include "search_feed.h"
#include "user_settings.h"
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
    Q_PROPERTY(QList<FavoriteFeedView> pinnedFeeds READ getPinnedFeeds NOTIFY pinnedFeedsChanged FINAL)
    Q_PROPERTY(QList<FavoriteFeedView> userOrderedPinnedFeeds READ getUserOrderedPinnedFeeds WRITE setUserOrderedPinnedFeeds NOTIFY userOrderedPinnedFeedsChanged FINAL)

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

    // Return mPinnedFeeds if mUserOrderedPinnedFeeds is empty
    const QList<FavoriteFeedView>& getUserOrderedPinnedFeeds() const;
    void setUserOrderedPinnedFeeds(const QList<FavoriteFeedView>& favorites);
    Q_INVOKABLE void clearUserOrderedPinnedFeed();

    Q_INVOKABLE FavoriteFeedView getPinnedFeed(const QString& uri) const;
    Q_INVOKABLE FavoriteFeedView getPinnedSearch(const QString& name) const;

    bool getUpdateSavedFeedsModelInProgress() const { return mUpdateSavedFeedsModelInProgress; }
    void setUpdateSavedFeedsModelInProgress(bool inProgress);

    Q_INVOKABLE FeedListModel* getSavedFeedsModel();
    Q_INVOKABLE void removeSavedFeedsModel();

    Q_INVOKABLE ListListModel* getSavedListsModel();
    Q_INVOKABLE void removeSavedListsModel();

    void saveTo(ATProto::UserPreferences& userPreferences, UserSettings& settings) const;

signals:
    void feedSaved();
    void listSaved();
    void feedPinned();
    void feedUnpinned(QString uri);
    void listPinned();
    void listUnpinned(QString uri);
    void searchPinned(QString name);
    void searchUnpinned(QString name);
    void updateSavedFeedsModelInProgressChanged();
    void pinnedFeedsChanged();
    void userOrderedPinnedFeedsChanged();

private:
    void addFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void addFeeds(QList<FavoriteFeedView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void addToUserOrderedPinnedFeeds(const FavoriteFeedView& favorite);
    void removeFromUserOrderedPinnedFeeds(const FavoriteFeedView& favorite);
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
    void saveSearchFeedsTo(UserSettings& settings) const;
    void cleanupSettings();

    template<typename Container>
    void removeNonPinnedFeeds(const Container& feedUris, const std::function<void(const QString& uri)>& removeFun);

    ATProto::UserPreferences::SavedFeedsPref mSavedFeedsPref;
    std::unordered_set<QString> mSavedUris;
    std::unordered_set<QString> mPinnedUris;
    std::unordered_set<QString> mPinnedSearches;
    QList<GeneratorView> mSavedFeeds; // sorted by name
    QList<ListView> mSavedLists; // sorted by name
    QList<FavoriteFeedView> mPinnedFeeds; // sorted by name

    // If this list is empty, then mPinnedFeeds is used for the UI
    QList<FavoriteFeedView> mUserOrderedPinnedFeeds; // ordered by the user

    int mSavedFeedsModelId = -1;
    int mSavedListsModelId = -1;
    bool mUpdateSavedFeedsModelInProgress = false;
    Skywalker* mSkywalker;
};

}
