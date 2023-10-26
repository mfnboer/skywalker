// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "profile_store.h"
#include "search_utils.h"

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
    qDebug() << "Add profile:" << profile.getDid() << profile.getHandle();
    const QString& did = profile.getDid();
    Q_ASSERT(!did.isEmpty());
    if (did.isEmpty())
        return;

    mDidProfileMap[did] = profile.nonVolatileCopy();
}

void ProfileStore::remove(const QString& did)
{
    qDebug() << "Remove profile:" << did;
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

void IndexedProfileStore::add(const BasicProfile& profile)
{
    ProfileStore::add(profile);
    addToIndex(profile);
}

void IndexedProfileStore::remove(const QString& did)
{
    const BasicProfile* profile = get(did);

    if (!profile)
        return;

    removeFromIndex(profile);
    ProfileStore::remove(did);
}

void IndexedProfileStore::clear()
{
    mWordIndex.clear();
    ProfileStore::clear();
}

const std::unordered_set<const BasicProfile*> IndexedProfileStore::findProfiles(const QString& wordPrefix, int maxWords) const
{
    std::unordered_set<const BasicProfile*> matches;
    const auto& its = mWordIndex.equal_range(wordPrefix);

    for (auto it = its.first; it != its.second; ++it)
    {
        for (const auto* profile : it->second)
        {
            matches.insert(profile);
            if (matches.size() >= maxWords)
                return matches;
        }
    }

    return matches;
}

std::set<QString> IndexedProfileStore::getWords(const BasicProfile& profile) const
{
    std::set<QString> words = SearchUtils::getWords(profile.getDisplayName());
    const QString handle = profile.getHandle();
    const int dotIndex = handle.indexOf('.');

    if (dotIndex < 0)
        words.insert(handle);
    else if (dotIndex > 0)
        words.insert(handle.sliced(0, dotIndex));

    return words;
}

void IndexedProfileStore::addToIndex(const BasicProfile& profile)
{
    const std::set<QString> words = getWords(profile);
    const BasicProfile* basicProfile = get(profile.getDid());

    Q_ASSERT(basicProfile);
    if (!basicProfile)
        return;

    for (const auto& word : words)
        mWordIndex[word].insert(basicProfile);
}

void IndexedProfileStore::removeFromIndex(const BasicProfile* profile)
{
    const std::set<QString> words = getWords(*profile);

    for (const auto& word : words)
    {
        auto& wordIndexSet = mWordIndex[word];
        wordIndexSet.erase(profile);

        if (wordIndexSet.empty())
            mWordIndex.erase(word);
    }
}

}
