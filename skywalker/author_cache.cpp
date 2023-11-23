// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_cache.h"

namespace Skywalker {

std::unique_ptr<AuthorCache> AuthorCache::sInstance;

AuthorCache::Entry::Entry(const BasicProfile& profile) :
    mAuthor(profile.nonVolatileCopy())
{
    Q_ASSERT(!mAuthor.isVolatile());
}

AuthorCache::AuthorCache() :
    mCache(1000)
{
}

void AuthorCache::clear()
{
    mCache.clear();
}

void AuthorCache::put(const BasicProfile& author)
{
    const QString& did = author.getDid();
    Q_ASSERT(!did.isEmpty());
    if (did.isEmpty())
        return;

    if (did == mUser.getDid())
        return;

    const auto* profile = getFromStores(did);
    if (profile)
        return;

    auto* entry = new Entry(author);
    mCache.insert(did, entry);
    qDebug() << "Cached:" << did << "size:" << mCache.size();
}

const BasicProfile* AuthorCache::get(const QString& did) const
{
    if (did == mUser.getDid())
        return &mUser;

    const auto* profile = getFromStores(did);
    if (profile)
        return profile;

    auto* entry = mCache[did];
    return entry ? &entry->getAuthor() : nullptr;
}

bool AuthorCache::contains(const QString& did) const
{
    return mCache.contains(did);
}

void AuthorCache::setUser(const BasicProfile& user)
{
    mUser = user.nonVolatileCopy();
}

void AuthorCache::addProfileStore(const IProfileStore* store)
{
    mProfileStores.insert(store);
}

const BasicProfile* AuthorCache::getFromStores(const QString& did) const
{
    for (const auto* store : mProfileStores)
    {
        auto* profile = store->get(did);
        if (profile)
            return profile;
    }

    return nullptr;
}

AuthorCache& AuthorCache::instance()
{
    if (!sInstance)
        sInstance = std::unique_ptr<AuthorCache>(new AuthorCache);

    return *sInstance;
}

}
