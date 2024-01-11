// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "favorite_feeds.h"
#include "skywalker.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker
{

static auto feedNameCompare = [](const GeneratorView& lhs, const GeneratorView& rhs){
                return QCollator().compare(lhs.getDisplayName(), rhs.getDisplayName()) < 0;
            };

static auto listNameCompare = [](const ListView& lhs, const ListView& rhs){
    return QCollator().compare(lhs.getName(), rhs.getName()) < 0;
};

static auto favoriteFeedNameCompare = [](const FavoriteFeedView& lhs, const FavoriteFeedView& rhs){
    return QCollator().compare(lhs.getName(), rhs.getName()) < 0;
};

FavoriteFeeds::FavoriteFeeds(Skywalker* skywalker, QObject* parent) :
    QObject(parent),
    mSkywalker(skywalker)
{
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
    mSavedFeeds.clear();
    mSavedLists.clear();
    mPinnedFeeds.clear();

    emit pinnedFeedsChanged();
}

void FavoriteFeeds::reset(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref)
{
    qDebug() << "Reset favorite feeds";
    clear();
    mSavedFeedsPref = savedFeedsPref;
    mSavedUris.insert(mSavedFeedsPref.mSaved.begin(), mSavedFeedsPref.mSaved.end());
    mPinnedUris.insert(mSavedFeedsPref.mPinned.begin(), mSavedFeedsPref.mPinned.end());
    updatePinnedViews();
}

void FavoriteFeeds::setSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators)
{
    setFeeds(mSavedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(savedGenerators));

    if (!mSavedFeeds.empty())
        updateSavedFeedsModel();
}

void FavoriteFeeds::setPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators)
{
    setFeeds(mPinnedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(pinnedGenerators));
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::addPinnedFeed(const ATProto::AppBskyGraph::ListView::SharedPtr& pinnedList)
{
    ListView listView(pinnedList);
    FavoriteFeedView view(listView);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::setFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators)
{
    feeds.clear();

    for (auto& gen : generators)
    {
        ATProto::AppBskyFeed::GeneratorView::SharedPtr sharedRaw(gen.release());
        GeneratorView view(sharedRaw);
        auto it = std::lower_bound(feeds.cbegin(), feeds.cend(), view, feedNameCompare);
        feeds.insert(it, view);
    }
}

void FavoriteFeeds::setFeeds(QList<FavoriteFeedView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators)
{
    feeds.clear();

    for (auto& gen : generators)
    {
        ATProto::AppBskyFeed::GeneratorView::SharedPtr sharedRaw(gen.release());
        GeneratorView generatorView(sharedRaw);
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

    if (mSavedFeedsModelId >= 0)
    {
        auto it = std::lower_bound(mSavedFeeds.cbegin(), mSavedFeeds.cend(), feed, feedNameCompare);
        mSavedFeeds.insert(it, feed);

        updateSavedFeedsModel();
    }

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

    if (mSavedFeedsModelId >= 0)
    {
        auto it2 = std::lower_bound(mSavedFeeds.cbegin(), mSavedFeeds.cend(), feed, feedNameCompare);

        if (it2 != mSavedFeeds.cend())
            mSavedFeeds.erase(it2);

        updateSavedFeedsModel();
    }

    emit feedSaved();
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

    emit feedPinned();
    emit pinnedFeedsChanged();
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
    auto it2 = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);

    if (it2 != mPinnedFeeds.cend())
        mPinnedFeeds.erase(it2);

    emit feedPinned();
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::addList(const ListView& list)
{
    if (isSavedFeed(list.getUri()))
    {
        qDebug() << "List already added:" << list.getName();
        return;
    }

    mSavedFeedsPref.mSaved.push_back(list.getUri());
    mSavedUris.insert(list.getUri());

    if (mSavedListsModelId >= 0)
    {
        auto it = std::lower_bound(mSavedLists.cbegin(), mSavedLists.cend(), list, listNameCompare);
        mSavedLists.insert(it, list);

        updateSavedListsModel();
    }

    emit listSaved();
}

void FavoriteFeeds::removeList(const ListView& list)
{
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

    if (mSavedListsModelId >= 0)
    {
        auto it2 = std::lower_bound(mSavedLists.cbegin(), mSavedLists.cend(), list, listNameCompare);

        if (it2 != mSavedLists.cend())
            mSavedLists.erase(it2);

        updateSavedListsModel();
    }

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

    emit listPinned();
    emit pinnedFeedsChanged();
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
    auto it2 = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);

    if (it2 != mPinnedFeeds.cend())
        mPinnedFeeds.erase(it2);

    emit listPinned();
    emit pinnedFeedsChanged();
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

    mSkywalker->getBskyClient()->getFeedGenerators(mSavedFeedsPref.mSaved,
        [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::Ptr output){
            setUpdateSavedFeedsModelInProgress(false);
            setSavedFeeds(std::move(output->mFeeds));
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
        [this, listUris](auto output){
            ATProto::AppBskyGraph::ListView::SharedPtr sharedListView(output->mList.release());

            if (sharedListView->mCreator->mDid != mSkywalker->getUserDid())
            {
                qDebug() << "Add saved list:" << sharedListView->mName;
                mSavedLists.append(ListView(sharedListView));
            }
            else
            {
                qDebug() << "Own list:" << sharedListView->mName;
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
    {
        updatePinnedGeneratorViews();
        updatePinnedListViews();
    }
}

void FavoriteFeeds::updatePinnedGeneratorViews()
{
    auto feedGeneratorUris = filterUris(mSavedFeedsPref.mPinned, ATProto::ATUri::COLLECTION_FEED_GENERATOR);

    if (feedGeneratorUris.empty())
        return;

    mSkywalker->getBskyClient()->getFeedGenerators(mSavedFeedsPref.mPinned,
        [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::Ptr output){
            setPinnedFeeds(std::move(output->mFeeds));
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Cannot get pinned feeds:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Cannot get pinned feeds: ") + msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void FavoriteFeeds::updatePinnedListViews()
{
    auto listUris = filterUris(mSavedFeedsPref.mSaved, ATProto::ATUri::COLLECTION_GRAPH_LIST);
    updatePinnedListViews(listUris);
}

void FavoriteFeeds::updatePinnedListViews(std::vector<QString> listUris)
{
    if (listUris.empty())
        return;

    const QString uri = listUris.back();
    listUris.pop_back();

    mSkywalker->getBskyClient()->getList(uri, 1, {},
        [this, listUris](auto output){
            ATProto::AppBskyGraph::ListView::SharedPtr sharedListView(output->mList.release());
            addPinnedFeed(sharedListView);
            updatePinnedListViews(std::move(listUris));
        },
        [this, listUris](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;
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
        mSavedListsModelId = mSkywalker->createListListModel(ListListModel::Type::LIST_PURPOSE_CURATE, "");
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

void FavoriteFeeds::saveTo(ATProto::UserPreferences& userPreferences) const
{
    userPreferences.setSavedFeedsPref(mSavedFeedsPref);
}

}
