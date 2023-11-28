// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "muted_words.h"
#include "search_utils.h"

namespace Skywalker {

MutedWords::MutedWords(QObject* parent) :
    QObject(parent)
{
}

QStringList MutedWords::getEntries() const
{
    QStringList sortedEntries;

    for (const auto& entry : mEntries)
        sortedEntries.append(entry.mRaw);

    sortedEntries.sort();
    return sortedEntries;
}

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

    emit entriesChanged();
}

void MutedWords::removeEntry(const QString& word)
{
    const auto it = std::find_if(mEntries.begin(), mEntries.end(),
                                 [&word](const auto& entry){ return entry.mRaw == word; });

    if (it == mEntries.end())
    {
        qDebug() << "Entry not found:" << word;
        return;
    }

    const int index = it - mEntries.begin();
    qDebug() << "Remove entry index:" << index << "word:" << word;
    removeEntry(index);
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

    emit entriesChanged();
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
