// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "muted_words.h"
#include "search_utils.h"

namespace Skywalker {

void MutedWords::addEntry(const QString& word)
{
    Entry entry;
    entry.mRaw = word;
    entry.mNormalizedWords = SearchUtils::getWords(word);

    mEntries.push_back(entry);
    const int entryIndex = (int)mEntries.size() - 1;

    if (entry.mNormalizedWords.size() == 1)
    {
        const QString& singleWord = entry.mNormalizedWords[0];
        addWordToIndex(singleWord, entryIndex, mSingleWordIndex);
    }
    else if (entry.mNormalizedWords.size() > 1)
    {
        const QString& firstWord = entry.mNormalizedWords[0];
        addWordToIndex(firstWord, entryIndex, mFirstWordIndex);
    }
}

void MutedWords::removeEntry(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < (int)mEntries.size());

    if (index < 0 || index >= (int)mEntries.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mEntries.size();
        return;
    }

    const Entry& entry = mEntries[index];

    if (entry.mNormalizedWords.size() == 1)
    {
        const QString& singleWord = entry.mNormalizedWords[0];
        removeWordFromIndex(singleWord, index, mSingleWordIndex);
    }
    else if (entry.mNormalizedWords.size() > 1)
    {
        const QString& firstWord = entry.mNormalizedWords[0];
        removeWordFromIndex(firstWord, index, mFirstWordIndex);
    }

    mEntries.erase(mEntries.begin() + index);
    reindexAfterRemoval(index, mSingleWordIndex);
    reindexAfterRemoval(index, mFirstWordIndex);
}

void MutedWords::addWordToIndex(const QString& word, int index, WordIndexType& wordIndex)
{
    wordIndex[word].insert(index);
}

void MutedWords::removeWordFromIndex(const QString& word, int index, WordIndexType& wordIndex)
{
    auto& indexEntry = wordIndex[word];
    indexEntry.erase(index);

    if (indexEntry.empty())
        wordIndex.erase(word);
}

void MutedWords::reindexAfterRemoval(int removedIndex, WordIndexType& wordIndex)
{
    for (auto& [_, indices] : wordIndex)
    {
        std::set<int> newIndices;
        for (int i : indices)
            newIndices.insert(i < removedIndex ? i : i - 1);

        indices = newIndices;
    }
}

bool MutedWords::match(const Post& post) const
{
    if (mEntries.empty())
        return false;

    const auto& uniquePostWords = post.getUniqueNormalizedWords();

    for (const auto& [word, _] : mSingleWordIndex)
    {
        if (uniquePostWords.count(word))
        {
            qDebug() << "Match on single word entry:" << word;
            return true;
        }
    }

    const auto& postWords = post.getNormalizedWords();

    for (const auto& [word, indices] : mFirstWordIndex)
    {
        const auto uniqueWordIt = uniquePostWords.find(word);

        if (uniqueWordIt == uniquePostWords.end())
            continue;

        qDebug() << "Matching first word:" << word;

        for (int mutedEntryIndex : indices)
        {
            Q_ASSERT(mutedEntryIndex < (int)mEntries.size());
            const auto& mutedEntry = mEntries[mutedEntryIndex];

            qDebug() << "Multi-word entry:" << mutedEntry.mRaw;

            for (int postWordIndex : uniqueWordIt->second)
            {
                int matchedWords = 0;

                for (int i = 0, j = postWordIndex;
                     i < (int)mutedEntry.mNormalizedWords.size() && j < (int)postWords.size();
                     ++i, ++j)
                {
                    if (mutedEntry.mNormalizedWords[i] != postWords[j])
                        break;

                    ++matchedWords;
                }

                if (matchedWords == (int)mutedEntry.mNormalizedWords.size())
                {
                    qDebug() << "Match on multi-word entry:" << mutedEntry.mRaw;
                    return true;
                }
            }
        }
    }

    return false;
}

}
