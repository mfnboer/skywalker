// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "songlink_cache.h"

namespace Skywalker {

std::unique_ptr<SonglinkCache> SonglinkCache::sInstance;

SonglinkCache::Entry::Entry(const SonglinkLinks& links) :
    mLinks(links)
{
}

void SonglinkCache::put(const QUrl& url, const SonglinkLinks& links)
{
    qDebug() << "Put:" << url;
    mCache.insert(url, new Entry(links));
}

const SonglinkLinks* SonglinkCache::get(const QUrl& url) const
{
    qDebug() << "Get:" << url;
    auto* entry = mCache[url];
    return entry ? &entry->getLinks() : nullptr;
}

bool SonglinkCache::contains(const QUrl& url) const
{
    return get(url) != nullptr;
}

SonglinkCache& SonglinkCache::instance()
{
    if (!sInstance)
        sInstance = std::unique_ptr<SonglinkCache>(new SonglinkCache);

    return *sInstance;
}

}
