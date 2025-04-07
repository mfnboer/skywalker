// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "wrapped_skywalker.h"
#include <atproto/lib/lexicon/app_bsky_feed.h>
#include <QCache>

namespace Skywalker {

// Cache of uri's from root posts with a thread indication.
class PostThreadCache : public WrappedSkywalker
{
    Q_OBJECT

public:
    static PostThreadCache& instance();

    void put(const QString& postUri, bool isThread);
    void putPost(const QString& uri);
    const bool* getIsThread(const QString& postUri) const;
    bool contains(const QString& postUri) const;

signals:
    void postAdded(const QString& uri);

private:
    explicit PostThreadCache(QObject* parent = nullptr);
    bool putThread(const ATProto::AppBskyFeed::ThreadElement::SharedPtr& thread);

    QCache<QString, bool> mCache{1000}; // post-uri -> isThread
    std::unordered_set<QString> mFetchingUris;
    std::unordered_set<QString> mFailedUris;

    static std::unique_ptr<PostThreadCache> sInstance;
};

}
