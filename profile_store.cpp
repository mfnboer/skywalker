// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile_store.h"

namespace Skywalker {

bool ProfileStore::contains(const QString& did) const
{
    return mDidProfileMap.count(did);
}

const BasicProfile* ProfileStore::get(const QString& did) const
{
    auto it = mDidProfileMap.find(did);
    return it != mDidProfileMap.end() ? &it->second : nullptr;
}

void ProfileStore::add(const BasicProfile& profile)
{
    const QString& did = profile.getDid();
    Q_ASSERT(!did.isEmpty());
    if (did.isEmpty())
        return;

    mDidProfileMap[did] = profile.nonVolatileCopy();
}

void ProfileStore::remove(const QString& did)
{
    mDidProfileMap.erase(did);
}

void ProfileStore::clear()
{
    mDidProfileMap.clear();
}

size_t ProfileStore::size()
{
    return mDidProfileMap.size();
}

}
