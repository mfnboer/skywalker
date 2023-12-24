// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "favorite_feeds.h"

namespace Skywalker
{

static auto feedNameCompare = [](const GeneratorView& lhs, const GeneratorView& rhs){
                return lhs.getDisplayName() < rhs.getDisplayName();
            };

FavoriteFeeds::FavoriteFeeds(QObject* parent) :
    QObject(parent)
{
}

void FavoriteFeeds::clear()
{
    qDebug() << "Clear favorite feeds";
    mSavedFeedsPref = {};
    mSavedUris.clear();
    mPinnedUris.clear();
    mSavedFeeds.clear();
    mPinnedFeeds.clear();
}

void FavoriteFeeds::reset(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref)
{
    qDebug() << "Reset favorite feeds";
    clear();
    mSavedFeedsPref = savedFeedsPref;
    mSavedUris.insert(mSavedFeedsPref.mSaved.begin(), mSavedFeedsPref.mSaved.end());
    mPinnedUris.insert(mSavedFeedsPref.mPinned.begin(), mSavedFeedsPref.mPinned.end());
}

void FavoriteFeeds::setSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators)
{
    setFeeds(mSavedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(savedGenerators));
}

void FavoriteFeeds::setPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators)
{
    setFeeds(mPinnedFeeds, std::forward<ATProto::AppBskyFeed::GeneratorViewList>(pinnedGenerators));
}

void FavoriteFeeds::setFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators)
{
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

    auto it = std::lower_bound(mSavedFeeds.cbegin(), mSavedFeeds.cend(), feed, feedNameCompare);
    mSavedFeeds.insert(it, feed);
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

    auto it2 = std::lower_bound(mSavedFeeds.cbegin(), mSavedFeeds.cend(), feed, feedNameCompare);
    mSavedFeeds.erase(it2);
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
    mPinnedFeeds.erase(it2);
}

}
