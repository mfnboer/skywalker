// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "author_cache.h"

namespace Skywalker {

std::unique_ptr<AuthorCache> AuthorCache::sInstance;

AuthorCache::Entry::Entry(const BasicProfile& profile) :
    mAuthor(profile)
{
}

AuthorCache::AuthorCache(QObject* parent) :
    WrappedSkywalker(parent),
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
}

void AuthorCache::putProfile(const QString& did, const std::function<void()>& addedCb)
{
    if (contains(did))
    {
        qDebug() << "Profile already in cache:" << did;
        return;
    }

    if (mFetchingDids.contains(did))
        return;

    if (!bskyClient())
        return;

    mFetchingDids.insert(did);

    bskyClient()->getProfile(did,
        [this, addedCb](auto profile){
            mFetchingDids.erase(profile->mDid);
            mFailedDids.erase(profile->mDid);
            put(BasicProfile(profile));
            emit profileAdded(profile->mDid);

            if (addedCb)
                addedCb();
        },
        [this, did](const QString& error, const QString& msg){
            qDebug() << "putProfile failed:" << did << error << " - " << msg;

            if (!mFailedDids.contains(did))
            {
                mFetchingDids.erase(did);
                mFailedDids.insert(did);
            }
            else
            {
                qWarning() << "Failed to get DID for the second time:" << did << error << " - " << msg;
                // Do not remove from mFetchingDids, so we will not try to get it again
            }
        });
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
    return get(did) != nullptr;
}

void AuthorCache::setUser(const BasicProfile& user)
{
    mUser = user;
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
