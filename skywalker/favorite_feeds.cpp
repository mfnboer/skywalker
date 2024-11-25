// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "favorite_feeds.h"
#include "skywalker.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker
{

static auto feedNameCompare = [](const GeneratorView& lhs, const GeneratorView& rhs){
    return lhs.getDisplayName().compare(rhs.getDisplayName(), Qt::CaseInsensitive) < 0;
};

static auto listNameCompare = [](const ListView& lhs, const ListView& rhs){
    return lhs.getName().compare(rhs.getName(), Qt::CaseInsensitive) < 0;
};

static auto favoriteFeedNameCompare = [](const FavoriteFeedView& lhs, const FavoriteFeedView& rhs){
    return lhs.getName().compare(rhs.getName(), Qt::CaseInsensitive) < 0;
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
}

void FavoriteFeeds::addPinnedFeed(const ATProto::AppBskyGraph::ListView::SharedPtr& pinnedList)
{
    qDebug() << "Add pinned list:" << pinnedList->mName;
    ListView listView(pinnedList);
    FavoriteFeedView view(listView);
    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), view, favoriteFeedNameCompare);
    mPinnedFeeds.insert(it, view);
    qInfo() << "Pinned:" << view.getName() << "size:" << mPinnedFeeds.size();
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
        auto it2 = std::find_if(mSavedFeeds.cbegin(), mSavedFeeds.cend(),
                            [uri=feed.getUri()](const auto& f){ return f.getUri() == uri; });

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
    qDebug() << "Pinned:" << view.getName();

    emit feedPinned();
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

    emit feedPinned();
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
        auto it2 = std::find_if(mSavedLists.cbegin(), mSavedLists.cend(),
                             [uri=list.getUri()](const auto& l){ return l.getUri() == uri; });

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
    qDebug() << "Pinned:" << view.getName();

    emit listPinned();
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

    emit listPinned();
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
    mPinnedFeeds.clear();

    if (!mSavedFeedsPref.mPinned.empty())
    {
        updatePinnedGeneratorViews();
        updatePinnedListViews();
    }
}

void FavoriteFeeds::updatePinnedGeneratorViews()
{
    auto feedGeneratorUris = filterUris(mSavedFeedsPref.mPinned, ATProto::ATUri::COLLECTION_FEED_GENERATOR);
    qDebug() << "Update pinned generators:" << feedGeneratorUris.size();

    if (feedGeneratorUris.empty())
        return;

    mSkywalker->getBskyClient()->getFeedGenerators(feedGeneratorUris,
        [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr output){
            addPinnedFeeds(std::move(output->mFeeds));
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Cannot get pinned feeds:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Cannot get pinned feeds: ") + msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

void FavoriteFeeds::updatePinnedListViews()
{
    auto listUris = filterUris(mSavedFeedsPref.mPinned, ATProto::ATUri::COLLECTION_GRAPH_LIST);
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
            addPinnedFeed(output->mList);
            updatePinnedListViews(std::move(listUris));
        },
        [this, listUris, uri](const QString& error, const QString& msg){
            qWarning() << error << " - " << msg;

            if (ATProto::Client::isListNotFoundError(error, msg))
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

void FavoriteFeeds::saveTo(ATProto::UserPreferences& userPreferences) const
{
    userPreferences.setSavedFeedsPref(mSavedFeedsPref);
}

}
