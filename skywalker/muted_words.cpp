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

    return sortedEntries;
}

void MutedWords::clear()
{
    mDirty = false;

    if (mEntries.empty())
        return;

    mEntries.clear();
    mSingleWordIndex.clear();
    mFirstWordIndex.clear();

    emit entriesChanged();
}

// A word can be a phrase..
static QString cleanRawWord(const QString& word)
{
    const auto& words = SearchUtils::getWords(word);
    QString cleanedWord;

    if (word.startsWith('#') && words.size() == 1)
        cleanedWord += '#';

    for (int i = 0; i < (int)words.size(); ++i)
    {
        if (i != 0)
            cleanedWord += ' ';

        cleanedWord += words[i];
    }

    return cleanedWord;
}

void MutedWords::addEntry(const QString& word)
{
    const auto& [it, inserted] = mEntries.emplace(cleanRawWord(word),
                                                  SearchUtils::getNormalizedWords(word));

    if (!inserted)
    {
        qDebug() << "Already muted:" << word;
        return;
    }

    const auto& entry = *it;

    if (entry.isHashtag())
        addWordToIndex(&entry, mHashTagIndex);
    else if (entry.mNormalizedWords.size() == 1)
        addWordToIndex(&entry, mSingleWordIndex);
    else if (entry.mNormalizedWords.size() > 1)
        addWordToIndex(&entry, mFirstWordIndex);

    mDirty = true;
    emit entriesChanged();
}

void MutedWords::removeEntry(const QString& word)
{
    const Entry searchEntry{ word, {} };
    const auto it = mEntries.find(searchEntry);

    if (it == mEntries.end())
    {
        qDebug() << "Entry not found:" << word;
        return;
    }

    const Entry& entry = *it;

    if (entry.isHashtag())
        removeWordFromIndex(&entry, mHashTagIndex);
    else if (entry.mNormalizedWords.size() == 1)
        removeWordFromIndex(&entry, mSingleWordIndex);
    else if (entry.mNormalizedWords.size() > 1)
        removeWordFromIndex(&entry, mFirstWordIndex);

    mEntries.erase(it);
    mDirty = true;
    emit entriesChanged();
}

void MutedWords::addWordToIndex(const Entry* entry, WordIndexType& wordIndex)
{
    Q_ASSERT(entry);
    Q_ASSERT(entry->mNormalizedWords.size() > 0);
    const QString& word = entry->mNormalizedWords[0];
    wordIndex[word].insert(entry);
}

void MutedWords::removeWordFromIndex(const Entry* entry, WordIndexType& wordIndex)
{
    Q_ASSERT(entry);
    Q_ASSERT(entry->mNormalizedWords.size() > 0);
    const QString& word = entry->mNormalizedWords[0];
    auto& indexEntry = wordIndex[word];
    indexEntry.erase(entry);

    if (indexEntry.empty())
        wordIndex.erase(word);
}

bool MutedWords::match(const NormalizedWordIndex& post) const
{
    if (mEntries.empty())
        return false;

    const auto& postHashtags = post.getUniqueHashtags();

    for (const auto& [word, _] : mHashTagIndex)
    {
        if (postHashtags.count(word))
        {
            qDebug() << "Match on hashtag:" << word;
            return true;
        }
    }

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

    for (const auto& [word, entries] : mFirstWordIndex)
    {
        const auto uniqueWordIt = uniquePostWords.find(word);

        if (uniqueWordIt == uniquePostWords.end())
            continue;

        qDebug() << "Matching first word:" << word;

        for (const Entry* mutedEntry : entries)
        {
            Q_ASSERT(mutedEntry);
            qDebug() << "Multi-word entry:" << mutedEntry->mRaw;

            for (int postWordIndex : uniqueWordIt->second)
            {
                int matchedWords = 0;

                for (int i = 0, j = postWordIndex;
                     i < (int)mutedEntry->mNormalizedWords.size() && j < (int)postWords.size();
                     ++i, ++j)
                {
                    if (mutedEntry->mNormalizedWords[i] != postWords[j])
                        break;

                    ++matchedWords;
                }

                if (matchedWords == (int)mutedEntry->mNormalizedWords.size())
                {
                    qDebug() << "Match on multi-word entry:" << mutedEntry->mRaw;
                    return true;
                }
            }
        }
    }

    return false;
}

void MutedWords::load(const UserSettings* userSettings)
{
    Q_ASSERT(userSettings);
    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return;
    }

    clear();
    const QStringList& mutedWords = userSettings->getMutedWords(did);

    for (const auto& word : mutedWords)
        addEntry(word);

    qDebug() << "Muted words loaded:" << mEntries.size();
    mDirty = false;
}

void MutedWords::save(UserSettings* userSettings)
{
    Q_ASSERT(userSettings);

    if (!mDirty)
        return;

    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return;
    }

    QStringList mutedWords;

    for (const auto& entry : mEntries)
        mutedWords.append(entry.mRaw);

    userSettings->saveMutedWords(did, mutedWords);
    qDebug() << "Muted words saved:" << mutedWords.size();
    mDirty = false;
}

bool MutedWords::noticeSeen(const UserSettings* userSettings) const
{
    Q_ASSERT(userSettings);
    return userSettings->getMutedWordsNoticeSeen();
}

void MutedWords::setNoticeSeen(UserSettings* userSettings, bool seen) const
{
    Q_ASSERT(userSettings);
    userSettings->setMutedWordsNoticeSeen(seen);
}

}
