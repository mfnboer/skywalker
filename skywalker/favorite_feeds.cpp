// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "favorite_feeds.h"
#include "search_utils.h"
#include "skywalker.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker
{

static auto feedNameCompare = [](const GeneratorView& lhs, const GeneratorView& rhs){
    return SearchUtils::normalizedCompare(lhs.getDisplayName(), rhs.getDisplayName()) < 0;
};

static auto listNameCompare = [](const ListView& lhs, const ListView& rhs){
    return SearchUtils::normalizedCompare(lhs.getName(), rhs.getName()) < 0;
};

static QString getFavoriteSortName(const FavoriteFeedView& favorite)
{
    if (favorite.getType() == QEnums::FAVORITE_SEARCH && favorite.getSearchFeed().isHashtag())
        return favorite.getName().sliced(1); // remove hash

    return favorite.getName();
}

static auto favoriteFeedNameCompare = [](const FavoriteFeedView& lhs, const FavoriteFeedView& rhs){
    const auto lhsName = getFavoriteSortName(lhs);
    const auto rhsName = getFavoriteSortName(rhs);

    return SearchUtils::normalizedCompare(lhsName, rhsName) < 0;
};

FavoriteFeeds::FavoriteFeeds(Skywalker* skywalker, QObject* parent) :
    QObject(parent),
    mSkywalker(skywalker)
{
    connect(this, &FavoriteFeeds::pinnedFeedsChanged, this, [this]{
        if (mUserOrderedPinnedFeeds.empty())
            emit userOrderedPinnedFeedsChanged();
    });
}

FavoriteFeeds::~FavoriteFeeds()
{
    removeSavedFeedsModel();
    removeSavedListsModel();
}

void FavoriteFeeds::clear()
{
    qDebug() << "Clear favorite feeds";
    mSavedFeedsPref = {};
    mSavedUris.clear();
    mPinnedUris.clear();
    mPinnedSearches.clear();
    mSavedFeeds.clear();
    mSavedLists.clear();
    mPinnedFeeds.clear();
    emit pinnedFeedsChanged();
    mUserOrderedPinnedFeedsInitialized = false;
    mUserOrderedPinnedFeeds.clear();
    emit userOrderedPinnedFeedsChanged();
}

void FavoriteFeeds::init(const SearchFeed::List& searchFeeds, const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref)
{
    qDebug() << "Initialize favorite feeds";
    clear();

    // NOTE: order is important, setting save feeds preferences is async and will eventually
    // initialize user ordered pinned feeds. For that the search feeds must have been set.
    set(searchFeeds);
    set(savedFeedsPref);
}

void FavoriteFeeds::set(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref)
{
    qDebug() << "Set favorite feeds";
    mSavedFeedsPref = savedFeedsPref;
    mSavedUris.insert(mSavedFeedsPref.mSaved.begin(), mSavedFeedsPref.mSaved.end());
    mPinnedUris.insert(mSavedFeedsPref.mPinned.begin(), mSavedFeedsPref.mPinned.end());
    updatePinnedViews();
}

void FavoriteFeeds::set(const SearchFeed::List& searchFeeds)
{
    qDebug() << "Set favorite search feeds";
    mPinnedSearches.clear();

    for (const auto& search : searchFeeds)
        pinSearch(search);
}

void FavoriteFeeds::initUserOrderedPinnedFeeds()
{
    qInfo() << "Init user ordered pinned feeds";
    mUserOrderedPinnedFeeds.clear();
    mUserOrderedPinnedFeedsInitialized = true;

    auto& settings = *mSkywalker->getUserSettings();
    const QString userDid = mSkywalker->getUserDid();
    const QStringList favoriteKeys = settings.getUserOrderedPinnedFeed(userDid);

    if (favoriteKeys.empty())
    {
        qInfo() << "No user ordered feeds";
        return;
    }

    for (const auto& key : favoriteKeys)
    {
        if (isPinnedFeed(key))
        {
            const auto favorite = getPinnedFeed(key);
            mUserOrderedPinnedFeeds.push_back(favorite);
        }
        else if (isPinnedSearch(key))
        {
            const auto favorite = getPinnedSearch(key);
            mUserOrderedPinnedFeeds.push_back(favorite);
        }
        else
        {
            // May have been unpinned by another client
            qInfo() << "Favorite not pinned:" << key;
        }
    }

    const std::unordered_set<QString> sortedKeys(favoriteKeys.begin(), favoriteKeys.end());

    // Pinned feeds may have been added with another client
    for (const auto& feed : std::as_const(mPinnedFeeds))
    {
        if (!sortedKeys.contains(feed.getKey()))
        {
            qDebug() << "New favorite:" << feed.getKey();
            mUserOrderedPinnedFeeds.push_back(feed);
        }
    }

    emit userOrderedPinnedFeedsChanged();
    saveUserOrderedPinnedFeeds();
}

void FavoriteFeeds::addSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators)
{
    qDebug() << "Add saved feeds:" << savedGenerators.size();
    addFeeds(mSavedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(savedGenerators));

    if (!mSavedFeeds.empty())
        updateSavedFeedsModel();
}

void FavoriteFeeds::addPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators)
{
    qDebug() << "Add pinned feeds:" << pinnedGenerators.size();
    addFeeds(mPinnedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(pinnedGenerators));
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::addPinnedFeed(const ATProto::AppBskyGraph::ListView::SharedPtr& pinnedList)
{
    qDebug() << "Add pinned list:" << pinnedList->mName;
    ListView listView(pinnedList);
    FavoriteFeedView view(listView);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    qInfo() << "Pinned:" << view.getName() << "size:" << mPinnedFeeds.size();
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::addFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators)
{
    for (auto& gen : generators)
    {
        GeneratorView view(gen);
        auto it = std::lower_bound(feeds.cbegin(), feeds.cend(), view, feedNameCompare);
        feeds.insert(it, view);
    }
}

void FavoriteFeeds::addFeeds(QList<FavoriteFeedView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators)
{
    for (auto& gen : generators)
    {
        GeneratorView generatorView(gen);
        FavoriteFeedView view(generatorView);
        auto it = std::lower_bound(feeds.cbegin(), feeds.cend(), view, favoriteFeedNameCompare);
        feeds.insert(it, view);
    }
}

void FavoriteFeeds::addFeed(const GeneratorView& feed)
{
    if (isSavedFeed(feed.getUri()))
    {
        qDebug() << "Feed already added:" << feed.getDisplayName();
        return;
    }

    mSavedFeedsPref.mSaved.push_back(feed.getUri());
    mSavedUris.insert(feed.getUri());

    auto it = std::lower_bound(mSavedFeeds.cbegin(), mSavedFeeds.cend(), feed, feedNameCompare);
    mSavedFeeds.insert(it, feed);
    updateSavedFeedsModel();

    emit feedSaved();
}

void FavoriteFeeds::removeFeed(const GeneratorView& feed)
{
    if (!isSavedFeed(feed.getUri()))
    {
        qDebug() << "Feed already removed:" << feed.getDisplayName();
        return;
    }

    // When a feed is removed it cannot be still pinned
    pinFeed(feed, false);

    auto it = std::find(mSavedFeedsPref.mSaved.cbegin(), mSavedFeedsPref.mSaved.cend(), feed.getUri());
    mSavedFeedsPref.mSaved.erase(it);
    mSavedUris.erase(feed.getUri());

    auto it2 = std::find_if(mSavedFeeds.cbegin(), mSavedFeeds.cend(),
                        [uri=feed.getUri()](const auto& f){ return f.getUri() == uri; });

    if (it2 != mSavedFeeds.cend())
        mSavedFeeds.erase(it2);

    updateSavedFeedsModel();

    emit feedSaved();
}

void FavoriteFeeds::addToUserOrderedPinnedFeeds(const FavoriteFeedView& favorite)
{
    if (mUserOrderedPinnedFeeds.empty())
        return;

    mUserOrderedPinnedFeeds.push_back(favorite);
    qDebug() << "Added to user ordered pinned feeds:" << favorite.getName();
    emit userOrderedPinnedFeedsChanged();
    saveUserOrderedPinnedFeeds();
}

void FavoriteFeeds::removeFromUserOrderedPinnedFeeds(const FavoriteFeedView& favorite)
{
    if (mUserOrderedPinnedFeeds.empty())
        return;

    auto removed = mUserOrderedPinnedFeeds.removeOne(favorite);
    qDebug() << "Removed from user ordered pinneds:" << favorite.getName() << "removed:" << removed;
    emit userOrderedPinnedFeedsChanged();
    saveUserOrderedPinnedFeeds();
}

void FavoriteFeeds::pinFeed(const GeneratorView& feed, bool pin)
{
    if (pin)
        pinFeed(feed);
    else
        unpinFeed(feed);
}

void FavoriteFeeds::pinFeed(const GeneratorView& feed)
{
    if (isPinnedFeed(feed.getUri()))
    {
        qDebug() << "Feed already pinned:" << feed.getDisplayName();
        return;
    }

    if (!isSavedFeed(feed.getUri()))
        addFeed(feed);

    mSavedFeedsPref.mPinned.push_back(feed.getUri());
    mPinnedUris.insert(feed.getUri());

    FavoriteFeedView view(feed);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    qDebug() << "Pinned:" << view.getName();

    emit feedPinned();
    emit pinnedFeedsChanged();
    addToUserOrderedPinnedFeeds(view);
}

void FavoriteFeeds::unpinFeed(const GeneratorView& feed)
{
    if (!isPinnedFeed(feed.getUri()))
    {
        qDebug() << "Feed not pinned:" << feed.getDisplayName();
        return;
    }

    auto it = std::find(mSavedFeedsPref.mPinned.cbegin(), mSavedFeedsPref.mPinned.cend(), feed.getUri());
    mSavedFeedsPref.mPinned.erase(it);
    mPinnedUris.erase(feed.getUri());

    FavoriteFeedView view(feed);
    auto it2 = std::find_if(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(),
                         [uri=feed.getUri()](const auto& f){ return f.getUri() == uri; });

    if (it2 != mPinnedFeeds.cend())
    {
        qDebug() << "Unpin:" << it2->getName();
        mPinnedFeeds.erase(it2);
    }

    emit feedUnpinned(feed.getUri());
    emit pinnedFeedsChanged();
    removeFromUserOrderedPinnedFeeds(view);
}

void FavoriteFeeds::addList(const ListView& list)
{
    qDebug() << "Add list:" << list.getName() << "model:" << mSavedListsModelId;

    if (isSavedFeed(list.getUri()))
    {
        qDebug() << "List already added:" << list.getName();
        return;
    }

    mSavedFeedsPref.mSaved.push_back(list.getUri());
    mSavedUris.insert(list.getUri());

    auto it = std::lower_bound(mSavedLists.cbegin(), mSavedLists.cend(), list, listNameCompare);
    mSavedLists.insert(it, list);
    updateSavedListsModel();

    emit listSaved();
}

void FavoriteFeeds::removeList(const ListView& list)
{
    qDebug() << "Remove list:" << list.getName();

    if (!isSavedFeed(list.getUri()))
    {
        qDebug() << "List already removed:" << list.getName();
        return;
    }

    // When a list is removed it cannot be still pinned
    pinList(list, false);

    auto it = std::find(mSavedFeedsPref.mSaved.cbegin(), mSavedFeedsPref.mSaved.cend(), list.getUri());
    mSavedFeedsPref.mSaved.erase(it);
    mSavedUris.erase(list.getUri());

    auto it2 = std::find_if(mSavedLists.cbegin(), mSavedLists.cend(),
                         [uri=list.getUri()](const auto& l){ return l.getUri() == uri; });

    if (it2 != mSavedLists.cend())
        mSavedLists.erase(it2);

    updateSavedListsModel();

    emit listSaved();
}

void FavoriteFeeds::pinList(const ListView& list, bool pin)
{
    if (pin)
        pinList(list);
    else
        unpinList(list);
}

void FavoriteFeeds::pinList(const ListView& list)
{
    if (isPinnedFeed(list.getUri()))
    {
        qDebug() << "List already pinned:" << list.getName();
        return;
    }

    if (!isSavedFeed(list.getUri()))
        addList(list);

    mSavedFeedsPref.mPinned.push_back(list.getUri());
    mPinnedUris.insert(list.getUri());

    FavoriteFeedView view(list);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    qDebug() << "Pinned:" << view.getName();

    emit listPinned();
    emit pinnedFeedsChanged();
    addToUserOrderedPinnedFeeds(view);
}

void FavoriteFeeds::unpinList(const ListView& list)
{
    if (!isPinnedFeed(list.getUri()))
    {
        qDebug() << "List not pinned:" << list.getName();
        return;
    }

    auto it = std::find(mSavedFeedsPref.mPinned.cbegin(), mSavedFeedsPref.mPinned.cend(), list.getUri());
    mSavedFeedsPref.mPinned.erase(it);
    mPinnedUris.erase(list.getUri());

    FavoriteFeedView view(list);
    auto it2 = std::find_if(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(),
                         [uri=list.getUri()](const auto& l){ return l.getUri() == uri; });

    if (it2 != mPinnedFeeds.cend())
    {
        qDebug() << "Unpin:" << it2->getName();
        mPinnedFeeds.erase(it2);
    }

    emit listUnpinned(list.getUri());
    emit pinnedFeedsChanged();
    removeFromUserOrderedPinnedFeeds(view);
}

void FavoriteFeeds::pinSearch(const SearchFeed& search, bool pin)
{
    if (pin)
        pinSearch(search);
    else
        unpinSearch(search);
}

void FavoriteFeeds::pinSearch(const SearchFeed& search)
{
    if (isPinnedSearch(search.getName()))
    {
        qDebug() << "Search already pinned:" << search.getName();
        return;
    }

    mPinnedSearches.insert(search.getName());

    FavoriteFeedView view(search);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    qDebug() << "Pinned:" << view.getName();

    emit searchPinned(view.getName());
    emit pinnedFeedsChanged();
    addToUserOrderedPinnedFeeds(view);
}

void FavoriteFeeds::unpinSearch(const SearchFeed& search)
{
    if (!isPinnedSearch(search.getName()))
    {
        qDebug() << "Search not pinned:" << search.getName();
        return;
    }

    mPinnedSearches.erase(search.getName());

    FavoriteFeedView view(search);
    auto it = std::find_if(
        mPinnedFeeds.cbegin(), mPinnedFeeds.cend(),
        [name=view.getName()](const auto& fav){
            return fav.getType() == QEnums::FAVORITE_SEARCH && fav.getName() == name;
        });

    if (it != mPinnedFeeds.cend())
    {
        qDebug() << "Unpin:" << it->getName();
        mPinnedFeeds.erase(it);
    }

    emit searchUnpinned(search.getName());
    emit pinnedFeedsChanged();
    removeFromUserOrderedPinnedFeeds(view);
}

const QList<FavoriteFeedView>& FavoriteFeeds::getUserOrderedPinnedFeeds() const
{
    return mUserOrderedPinnedFeeds.empty() ? mPinnedFeeds : mUserOrderedPinnedFeeds;
}

void FavoriteFeeds::setUserOrderedPinnedFeeds(const QList<FavoriteFeedView>& favorites)
{
    if (favorites == mPinnedFeeds)
    {
        qDebug() << "User ordered feeds are sorted";
        clearUserOrderedPinnedFeed();
        return;
    }

    if (favorites == mUserOrderedPinnedFeeds)
    {
        qDebug() << "User ordered feeds are not changed";
        return;
    }

    mUserOrderedPinnedFeeds = favorites;
    emit userOrderedPinnedFeedsChanged();
    saveUserOrderedPinnedFeeds();
}

void FavoriteFeeds::clearUserOrderedPinnedFeed()
{
    if (mUserOrderedPinnedFeeds.empty())
        return;

    mUserOrderedPinnedFeeds.clear();
    emit userOrderedPinnedFeedsChanged();
    saveUserOrderedPinnedFeeds();
}

FavoriteFeedView FavoriteFeeds::getPinnedFeed(const QString& uri) const
{
    for (const auto& feed : mPinnedFeeds)
    {
        if (feed.getUri() == uri)
            return feed;
    }

    return {};
}

FavoriteFeedView FavoriteFeeds::getPinnedSearch(const QString& name) const
{
    for (const auto& feed : mPinnedFeeds)
    {
        if (feed.getName() == name)
            return feed;
    }

    return {};
}

std::vector<QString> FavoriteFeeds::filterUris(const std::vector<QString> uris, char const* collection) const
{
    std::vector<QString> filtered;

    for (const auto& uri : uris)
    {
        ATProto::ATUri atUri(uri);

        if (atUri.isValid() && atUri.getCollection() == collection)
            filtered.push_back(uri);
    }

    return filtered;
}

void FavoriteFeeds::updateSavedViews()
{
    mSavedFeeds.clear();

    if (!mSavedFeedsPref.mSaved.empty())
    {
        updateSavedGeneratorViews();
        updateSavedListViews();
    }
}

void FavoriteFeeds::updateSavedGeneratorViews()
{
    auto feedGeneratorUris = filterUris(mSavedFeedsPref.mSaved, ATProto::ATUri::COLLECTION_FEED_GENERATOR);

    if (feedGeneratorUris.empty())
        return;

    setUpdateSavedFeedsModelInProgress(true);

    mSkywalker->getBskyClient()->getFeedGenerators(feedGeneratorUris,
        [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr output){
            setUpdateSavedFeedsModelInProgress(false);
            addSavedFeeds(std::move(output->mFeeds));
        },
        [this](const QString& error, const QString& msg){
            setUpdateSavedFeedsModelInProgress(false);
            qWarning() << "Cannot get saved feeds:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Cannot get saved feeds: ") + msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void FavoriteFeeds::updateSavedListViews()
{
    auto listUris = filterUris(mSavedFeedsPref.mSaved, ATProto::ATUri::COLLECTION_GRAPH_LIST);
    setUpdateSavedFeedsModelInProgress(true);
    updateSavedListViews(std::move(listUris));
}

void FavoriteFeeds::updateSavedListViews(std::vector<QString> listUris)
{
    if (listUris.empty())
    {
        setUpdateSavedFeedsModelInProgress(false);

        if (!mSavedLists.empty())
            updateSavedListsModel();

        return;
    }

    const QString uri = listUris.back();
    listUris.pop_back();

    mSkywalker->getBskyClient()->getList(uri, 1, {},
        [this, listUris](auto output){\
            if (output->mList->mCreator->mDid != mSkywalker->getUserDid())
            {
                qDebug() << "Add saved list:" << output->mList->mName;
                ListView view(output->mList);
                auto it = std::lower_bound(mSavedLists.cbegin(), mSavedLists.cend(), view, listNameCompare);
                mSavedLists.insert(it, view);
            }
            else
            {
                qDebug() << "Own list:" << output->mList->mName;
            }

            updateSavedListViews(std::move(listUris));
        },
        [this, listUris](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
            updateSavedListViews(std::move(listUris));
        });
}

void FavoriteFeeds::updatePinnedViews()
{
    if (!mSavedFeedsPref.mPinned.empty())
        updatePinnedGeneratorViews(); // NOTE: this will update pinned list views after it finished
    else
        initUserOrderedPinnedFeeds();

    emit pinnedFeedsChanged();
}

void FavoriteFeeds::updatePinnedGeneratorViews()
{
    auto feedGeneratorUris = filterUris(mSavedFeedsPref.mPinned, ATProto::ATUri::COLLECTION_FEED_GENERATOR);
    qDebug() << "Update pinned generators:" << feedGeneratorUris.size();

    if (feedGeneratorUris.empty()) {
        updatePinnedListViews();
        return;
    }

    mSkywalker->getBskyClient()->getFeedGenerators(feedGeneratorUris,
        [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr output){
            addPinnedFeeds(std::move(output->mFeeds));
            updatePinnedListViews();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Cannot get pinned feeds:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Cannot get pinned feeds: ") + msg, QEnums::STATUS_LEVEL_ERROR);
            updatePinnedListViews();
        });
}

void FavoriteFeeds::updatePinnedListViews()
{
    auto listUris = filterUris(mSavedFeedsPref.mPinned, ATProto::ATUri::COLLECTION_GRAPH_LIST);
    qDebug() << "Update pinned list views:" << listUris.size();
    updatePinnedListViews(listUris);
}

void FavoriteFeeds::updatePinnedListViews(std::vector<QString> listUris)
{
    if (listUris.empty())
    {
        initUserOrderedPinnedFeeds();
        cleanupSettings();
        return;
    }

    const QString uri = listUris.back();
    listUris.pop_back();

    mSkywalker->getBskyClient()->getList(uri, 1, {},
        [this, listUris](auto output){
            qDebug() << "Add list:" << output->mList->mUri << output->mList->mName;
            addPinnedFeed(output->mList);
            updatePinnedListViews(std::move(listUris));
        },
        [this, listUris, uri](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;

            if (ATProto::Client::isListNotFoundError(error))
            {
                qDebug() << "Remove unknown list:" << uri;

                mPinnedUris.erase(uri);
                auto itPinned = std::find(mSavedFeedsPref.mPinned.cbegin(), mSavedFeedsPref.mPinned.cend(), uri);
                if (itPinned != mSavedFeedsPref.mPinned.cend())
                    mSavedFeedsPref.mPinned.erase(itPinned);

                mSavedUris.erase(uri);
                auto itSaved = std::find(mSavedFeedsPref.mSaved.cbegin(), mSavedFeedsPref.mSaved.cend(), uri);
                if (itSaved != mSavedFeedsPref.mSaved.cend())
                    mSavedFeedsPref.mSaved.erase(itSaved);
            }
            else
            {
                mSkywalker->showStatusMessage(tr("Cannot get pinned list: ") + msg, QEnums::STATUS_LEVEL_ERROR);
            }

            updatePinnedListViews(std::move(listUris));
        });
}

void FavoriteFeeds::updateSavedFeedsModel()
{
    if (mSavedFeedsModelId < 0)
        return;

    auto* model = mSkywalker->getFeedListModel(mSavedFeedsModelId);
    model->clear();

    if (!mSavedFeeds.empty())
        model->addFeeds(mSavedFeeds);
    else
        updateSavedViews();
}

FeedListModel* FavoriteFeeds::getSavedFeedsModel()
{
    if (mSavedFeedsModelId < 0)
    {
        mSavedFeedsModelId = mSkywalker->createFeedListModel();
        updateSavedFeedsModel();
    }

    return mSkywalker->getFeedListModel(mSavedFeedsModelId);
}

void FavoriteFeeds::removeSavedFeedsModel()
{
    if (mSavedFeedsModelId >= 0)
    {
        mSkywalker->removeFeedListModel(mSavedFeedsModelId);
        mSavedFeedsModelId = -1;
    }
}

void FavoriteFeeds::updateSavedListsModel()
{
    if (mSavedListsModelId < 0)
        return;

    auto* model = mSkywalker->getListListModel(mSavedListsModelId);
    model->clear();

    if (!mSavedLists.empty())
        model->addLists(mSavedLists);
    else
        updateSavedViews();
}

ListListModel* FavoriteFeeds::getSavedListsModel()
{
    if (mSavedListsModelId < 0)
    {
        mSavedListsModelId = mSkywalker->createListListModel(
                ListListModel::Type::LIST_TYPE_SAVED,
                ListListModel::Purpose::LIST_PURPOSE_CURATE, "");
        updateSavedListsModel();
    }

    return mSkywalker->getListListModel(mSavedListsModelId);
}

void FavoriteFeeds::removeSavedListsModel()
{
    if (mSavedListsModelId >= 0)
    {
        mSkywalker->removeListListModel(mSavedListsModelId);
        mSavedListsModelId = -1;
    }
}

void FavoriteFeeds::setUpdateSavedFeedsModelInProgress(bool inProgress)
{
    if (mUpdateSavedFeedsModelInProgress != inProgress)
    {
        mUpdateSavedFeedsModelInProgress = inProgress;
        emit updateSavedFeedsModelInProgressChanged();
    }
}

void FavoriteFeeds::saveTo(ATProto::UserPreferences& userPreferences, UserSettings& settings) const
{
    userPreferences.setSavedFeedsPref(mSavedFeedsPref);
    saveSearchFeedsTo(settings);
}

void FavoriteFeeds::saveSearchFeedsTo(UserSettings& settings) const
{
    SearchFeed::List searchFeeds;

    for (const auto& favorite : mPinnedFeeds)
    {
        if (favorite.getType() == QEnums::FAVORITE_SEARCH)
            searchFeeds.push_back(favorite.getSearchFeed());
    }

    settings.setPinnedSearchFeeds(mSkywalker->getUserDid(), searchFeeds);
}

void FavoriteFeeds::saveUserOrderedPinnedFeeds() const
{
    if (!mUserOrderedPinnedFeedsInitialized)
        return;

    auto& settings = *mSkywalker->getUserSettings();
    const QString userDid = mSkywalker->getUserDid();
    QStringList keys;

    for (const auto& favorite : mUserOrderedPinnedFeeds)
    {
        const QString key = favorite.getKey();

        if (!key.isEmpty())
            keys.push_back(key);
        else
            qWarning() << "Empty key for favorite:" << favorite.getName();
    }

    qDebug() << "Save user ordered settings:" << keys;
    settings.setUserOrderedPinnedFeeds(userDid, keys);
}

void FavoriteFeeds::cleanupSettings()
{
    qDebug() << "Cleanup settings";
    auto& settings = *mSkywalker->getUserSettings();
    const QString userDid = mSkywalker->getUserDid();

    qDebug() << "Cleanup feed sync settings";
    const auto syncFeeds = settings.getSyncFeeds(userDid);
    removeNonPinnedFeeds(syncFeeds,
        [&settings, &userDid](const QString& uri){ settings.removeSyncFeed(userDid, uri); });

    qDebug() << "Cleanup feed view mode settings";
    removeNonPinnedFeeds(settings.getFeedViewModeUris(userDid),
        [&settings, &userDid](const QString& uri){ settings.setFeedViewMode(userDid, uri, QEnums::CONTENT_MODE_UNSPECIFIED); });

    qDebug() << "Cleanup feed hide replies settings";
    removeNonPinnedFeeds(settings.getFeedHideRepliesUris(userDid),
        [&settings, &userDid](const QString& uri){ settings.setFeedHideReplies(userDid, uri, false); });
}

template<typename Container>
void FavoriteFeeds::removeNonPinnedFeeds(const Container& feedUris, const std::function<void(const QString& uri)>& removeFun)
{
    for (const QString& uri : feedUris)
    {
        qDebug() << "Check:" << uri;
        if (!isPinnedFeed(uri))
        {
            qWarning() << "Feed not pinned:" << uri;
            removeFun(uri);
        }
    }
}

}
