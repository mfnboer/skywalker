// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "favorite_feeds.h"
#include "skywalker.h"

namespace Skywalker
{

static auto feedNameCompare = [](const GeneratorView& lhs, const GeneratorView& rhs){
                return QCollator().compare(lhs.getDisplayName(), rhs.getDisplayName()) < 0;
            };

FavoriteFeeds::FavoriteFeeds(Skywalker* skywalker, QObject* parent) :
    QObject(parent),
    mSkywalker(skywalker)
{
}

FavoriteFeeds::~FavoriteFeeds()
{
    removeSavedFeedsModel();
}

void FavoriteFeeds::clear()
{
    qDebug() << "Clear favorite feeds";
    mSavedFeedsPref = {};
    mSavedUris.clear();
    mPinnedUris.clear();
    mSavedFeeds.clear();
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

    auto it = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), feed, feedNameCompare);
    mPinnedFeeds.insert(it, feed);

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

    auto it2 = std::lower_bound(mPinnedFeeds.cbegin(), mPinnedFeeds.cend(), feed, feedNameCompare);

    if (it2 != mPinnedFeeds.cend())
        mPinnedFeeds.erase(it2);

    emit feedPinned();
    emit pinnedFeedsChanged();
}

void FavoriteFeeds::updateSavedViews()
{
    if (!mSavedFeedsPref.mSaved.empty())
    {
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
}

void FavoriteFeeds::updatePinnedViews()
{
    if (!mSavedFeedsPref.mPinned.empty())
    {
        mSkywalker->getBskyClient()->getFeedGenerators(mSavedFeedsPref.mPinned,
            [this](ATProto::AppBskyFeed::GetFeedGeneratorsOutput::Ptr output){
                setPinnedFeeds(std::move(output->mFeeds));
            },
            [this](const QString& error, const QString& msg){
                qWarning() << "Cannot get pinned feeds:" << error << " - " << msg;
                mSkywalker->showStatusMessage(tr("Cannot get pinned feeds: ") + msg, QEnums::STATUS_LEVEL_ERROR);
                // TODO: handle favorite feeds
            });
    }
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
