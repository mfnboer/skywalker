// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "hashtag_index.h"
#include "search_utils.h"

namespace Skywalker {

HashtagIndex::Entry::Entry(HashtagIndex* index, const QString& hashtag) :
    mHashtagIndex(index),
    mHashtag(hashtag),
    mNormalized(SearchUtils::normalizeText(hashtag))
{
}

HashtagIndex::Entry::~Entry()
{
    if (mHashtagIndex)
        mHashtagIndex->removeFromIndex(mHashtag, mNormalized);
}

HashtagIndex::HashtagIndex(int maxEntries) :
    mCache(maxEntries)
{
}

void HashtagIndex::clear()
{
    mCache.clear();
    mIndex.clear();
    setDirty(false);
}

void HashtagIndex::insert(const QString& hashtag)
{
    auto* entry = new Entry(this, hashtag);
    const QString normalized = entry->getNormalized();
    mCache.insert(hashtag, entry);

    // Adding to index must be done after insert in cache.
    // If the entry already exists, then the cache will destroy the new entry which triggers
    // removed from the index. The index not be removed as there is already an entry in the
    // cache. addToIndex will re-insert in that case.
    addToIndex(hashtag, normalized);
    setDirty(true);
}

void HashtagIndex::insert(const QStringList& hashtags)
{
    for (const auto& tag : hashtags)
        insert(tag);
}

static void addToResult(const QString& match, QStringList& result, std::unordered_set<QString>& alreadyFound)
{
    if (!alreadyFound.count(match))
    {
        result.push_back(match);
        alreadyFound.insert(match);
    }
}

QStringList HashtagIndex::find(const QString& hashtag, int limit, const QStringList& suppress) const
{
    qDebug() << "Find hashtag:" << hashtag << "limit:" << limit;

    QStringList result;
    result.reserve(limit);
    std::unordered_set<QString> alreadyFound;
    alreadyFound.insert(suppress.begin(), suppress.end());

    if (mCache.contains(hashtag))
        addToResult(hashtag, result, alreadyFound);

    if (result.size() >= limit)
        return result;

    const auto normalized = SearchUtils::normalizeText(hashtag);
    findFullMatch(normalized, limit, result, alreadyFound);

    if (result.size() >= limit)
        return result;

    findPrefixMatch(normalized, hashtag, limit, result, alreadyFound);
    return result;
}

void HashtagIndex::addToIndex(const QString& hashtag, const QString& normalized)
{
    mIndex[normalized].insert(hashtag);
}

void HashtagIndex::removeFromIndex(const QString& hashtag, const QString& normalized)
{
    auto& hashtags = mIndex[normalized];
    hashtags.erase(hashtag);

    if (hashtags.empty())
        mIndex.erase(normalized);
}

void HashtagIndex::findFullMatch(const QString& normalized, int limit, QStringList& result,
                                 std::unordered_set<QString>& alreadyFound) const
{
    auto it = mIndex.find(normalized);

    if (it == mIndex.end())
        return;

    for (const auto& hashtag : it->second)
    {
        addToResult(hashtag, result, alreadyFound);

        if (result.size() >= limit)
            return;
    }
}

void HashtagIndex::findPrefixMatch(const QString& normalized, const QString& nonNormalized, int limit, QStringList& result,
                                   std::unordered_set<QString>& alreadyFound) const
{
    size_t initialSize = result.size();

    for (auto it = mIndex.lower_bound(normalized);
         it != mIndex.end() && it->first.startsWith(normalized);
         ++it)
    {
        for (const auto& hashtag : it->second)
        {
            if (hashtag.startsWith(nonNormalized))
            {
                // Exact prefix match should higher up in the results
                if (!alreadyFound.count(hashtag))
                {
                    alreadyFound.insert(hashtag);
                    result.insert(result.begin() + initialSize, hashtag);
                    ++initialSize;
                }
            }
            else
            {
                addToResult(hashtag, result, alreadyFound);
            }

            if (result.size() >= limit)
                return;
        }
    }
}

}
