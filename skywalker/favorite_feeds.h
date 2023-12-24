// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "generator_view.h"
#include <atproto/lib/user_preferences.h>
#include <QObject>
#include <unordered_set>

namespace Skywalker {

class FavoriteFeeds : public QObject
{
    Q_OBJECT

public:
    explicit FavoriteFeeds(QObject* parent = nullptr);

    void clear();
    void reset(const ATProto::UserPreferences::SavedFeedsPref& savedFeedsPref);
    void setSavedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& savedGenerators);
    void setPinnedFeeds(ATProto::AppBskyFeed::GeneratorViewList&& pinnedGenerators);

    bool isSavedFeed(const QString& uri) const { return mSavedUris.count(uri); }
    bool isPinnedFeed(const QString& uri) const { return mPinnedUris.count(uri); }

    void addFeed(const GeneratorView& feed);
    void removeFeed(const GeneratorView& feed);
    void pinFeed(const GeneratorView& feed, bool pin);

    const QList<GeneratorView>& getPinnedFeeds() const { return mPinnedFeeds; }

private:
    void setFeeds(QList<GeneratorView>& feeds, ATProto::AppBskyFeed::GeneratorViewList&& generators);
    void pinFeed(const GeneratorView& feed);
    void unpinFeed(const GeneratorView& feed);

    ATProto::UserPreferences::SavedFeedsPref mSavedFeedsPref;
    std::unordered_set<QString> mSavedUris;
    std::unordered_set<QString> mPinnedUris;
    QList<GeneratorView> mSavedFeeds; // sorted by name
    QList<GeneratorView> mPinnedFeeds; // sorted by name
};

}
