// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_cache.h"

namespace Skywalker {

PostCache::Entry::Entry(const Post& post) :
    mPost(post)
{
}

PostCache::PostCache() :
    mCache(500)
{}

void PostCache::clear()
{
    mCache.clear();
}

void PostCache::put(const Post& post)
{
    auto* entry = new Entry(post);
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
