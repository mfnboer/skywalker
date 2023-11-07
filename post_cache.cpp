// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_cache.h"

namespace Skywalker {

PostCache::Entry::Entry(const ATProto::AppBskyFeed::PostView::SharedPtr& rawPostView, const Post& post) :
    mRawPostView(rawPostView),
    mPost(post)
{
    Q_ASSERT(mRawPostView);
}

PostCache::PostCache() :
    mCache(500)
{}

void PostCache::clear()
{
    mCache.clear();
}

void PostCache::put(const ATProto::AppBskyFeed::PostView::SharedPtr& rawPostView, const Post& post)
{
    auto* entry = new Entry(rawPostView, post);
    mCache.insert(post.getUri(), entry);
    qDebug() << "Cached:" << post.getUri() << "size:" << mCache.size();
}

const Post* PostCache::get(const QString& uri) const
{
    auto* entry = mCache[uri];
    return entry ? &entry->getPost() : nullptr;
}

bool PostCache::contains(const QString& uri) const
{
    return mCache.contains(uri);
}

std::vector<QString> PostCache::getNonCachedUris(const std::vector<QString>& uris) const
{
    std::vector<QString> nonCached;

    for (const auto& uri : uris)
    {
        if (!contains(uri))
            nonCached.push_back(uri);
    }

    return nonCached;
}

}
