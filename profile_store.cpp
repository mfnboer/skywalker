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
    mProfileWords.clear();
    ProfileStore::clear();
}

const std::unordered_set<const BasicProfile*> IndexedProfileStore::findProfiles(const QString& text, int limit) const
{
    const std::vector<QString> words = SearchUtils::getWords(text);

    if (words.empty())
        return {};

    if (words.size() == 1)
        return findWordPrefixMatch(words.front());

    std::unordered_set<const BasicProfile*> matches = findWordMatch(words.front());
    std::unordered_set<QString> usedWords = {words.front()};

    for (int i = 1; i < words.size() - 1; ++i)
    {
        const QString& word = words[i];

        if (usedWords.count(word))
            continue;

        removeNonWordMatches(matches, word);
        usedWords.insert(word);
    }

    removeNonPrefixMatches(matches, words.back());
    return matches;
}

void IndexedProfileStore::removeNonWordMatches(std::unordered_set<const BasicProfile*>& matches, const QString& word) const
{
    for (auto it = matches.begin(); it != matches.end(); )
    {
        const auto itProfile = mProfileWords.find(*it);

        Q_ASSERT(itProfile != mProfileWords.end());
        if (itProfile == mProfileWords.end())
            continue;

        const std::set<QString>& profileWords = itProfile->second;

        if (!profileWords.count(word))
            it = matches.erase(it);
        else
            ++it;
    }
}

void IndexedProfileStore::removeNonPrefixMatches(std::unordered_set<const BasicProfile*>& matches, const QString& prefix) const
{
    for (auto it = matches.begin(); it != matches.end(); )
    {
        const auto itProfile = mProfileWords.find(*it);

        Q_ASSERT(itProfile != mProfileWords.end());
        if (itProfile == mProfileWords.end())
            continue;

        const std::set<QString>& profileWords = itProfile->second;
        const auto wordIt = profileWords.lower_bound(prefix);

        if (wordIt != profileWords.end() && wordIt->startsWith(prefix))
            ++it;
        else
            it = matches.erase(it);
    }
}

const std::unordered_set<const BasicProfile*>& IndexedProfileStore::findWordMatch(const QString& word) const
{
    static const std::unordered_set<const BasicProfile*> NO_MATCH;

    const auto it = mWordIndex.find(word);
    return it != mWordIndex.end() ? it->second : NO_MATCH;
}

const std::unordered_set<const BasicProfile*> IndexedProfileStore::findWordPrefixMatch(const QString& prefix, int limit) const
{
    std::unordered_set<const BasicProfile*> matches;
    const auto& exactMatches = findWordMatch(prefix);

    if (exactMatches.size() >= limit)
    {
        auto exactIt = exactMatches.begin();

        for (int i = 0; i < limit; ++i)
            matches.insert(*(exactIt++));

        return matches;
    }

    matches = exactMatches;

    for (auto it = mWordIndex.lower_bound(prefix);
         it != mWordIndex.end() && it->first.startsWith(prefix);
         ++it)
    {
        for (const BasicProfile* profile : it->second)
        {
            matches.insert(profile);

            if (matches.size() >= limit)
                return matches;
        }
    }

    return matches;
}

std::set<QString> IndexedProfileStore::getWords(const BasicProfile& profile) const
{
    const std::vector<QString> wordList = SearchUtils::getWords(profile.getDisplayName());
    std::set<QString> words = std::set<QString>(wordList.begin(), wordList.end());
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
    std::set<QString> words = getWords(profile);
    const BasicProfile* basicProfile = get(profile.getDid());

    Q_ASSERT(basicProfile);
    if (!basicProfile)
        return;

    for (const auto& word : words)
        mWordIndex[word].insert(basicProfile);

    mProfileWords[basicProfile] = std::move(words);
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

    mProfileWords.erase(profile);
}

}
