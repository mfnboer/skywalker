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

bool MutedWords::preAdd(const Entry& entry)
{
    if (entry.wordCount() != 1)
        return true;

    if (entry.mRaw.startsWith('#'))
    {
        if (containsEntry(entry.mRaw.sliced(1)))
        {
            qDebug() << "Full content already muted for:" << entry.mRaw;
            return false;
        }
    }
    else
    {
        const QString hashtag = QString("#%1").arg(entry.mRaw);
        if (containsEntry(hashtag))
        {
            qDebug() << "Hashtag already muted for:" << entry.mRaw;
            removeEntry(hashtag);
        }
    }

    return true;
}

void MutedWords::addEntry(const QString& word)
{
    Entry newEntry(cleanRawWord(word), SearchUtils::getNormalizedWords(word));

    if (!preAdd(newEntry))
        return;

    const auto& [it, inserted] = mEntries.emplace(std::move(newEntry));

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

bool MutedWords::containsEntry(const QString& word)
{
    const Entry searchEntry{ word, {} };
    return mEntries.count(searchEntry);
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

bool MutedWords::legacyLoad(const UserSettings* userSettings)
{
    Q_ASSERT(userSettings);
    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return false;
    }

    // Do not clear existing muted words, but load them on top.
    // Legacy load is only called once during migration to bsky settings.
    const QStringList& mutedWords = userSettings->getMutedWords(did);

    if (mutedWords.isEmpty())
    {
        qDebug() << "No muted words to load from local app settings.";
        return false;
    }

    for (const auto& word : mutedWords)
        addEntry(word);

    qDebug() << "Muted words loaded from local app settings:" << mEntries.size();
    mDirty = true;
    return true;
}

void MutedWords::load(const ATProto::UserPreferences& userPrefs)
{
    clear();
    const auto& mutedWords = userPrefs.getMutedWordsPref();

    for (const auto& mutedWord : mutedWords.mItems)
    {
        bool matchContent = false;
        bool matchTag = false;

        for (const auto& target : mutedWord.mTargets)
        {
            switch (target.mTarget)
            {
            case ATProto::AppBskyActor::MutedWordTarget::CONTENT:
                matchContent = true;
                break;
            case ATProto::AppBskyActor::MutedWordTarget::TAG:
                matchTag = true;
                break;
            case ATProto::AppBskyActor::MutedWordTarget::UNKNOWN:
                break;
            }
        }

        if  (matchTag && !matchContent)
            addEntry(QString("#%1").arg(mutedWord.mValue));
        else
            addEntry(mutedWord.mValue);

    }

    qDebug() << "Muted words loaded:" << mEntries.size();
    mDirty = false;
}

void MutedWords::legacySave(UserSettings* userSettings)
{
    Q_ASSERT(false); // should not be used.
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

void MutedWords::save(ATProto::UserPreferences& userPrefs)
{
    if (!mDirty)
        return;

    ATProto::UserPreferences::MutedWordsPref& mutedWordsPrefs = userPrefs.getMutedWordsPref();
    mutedWordsPrefs.mItems.clear();

    // TODO: if any MutedWord properties get added in the future, then these get lost
    // as long as this function is not updated. We could load/save the raw JSON object.
    for (const auto& entry : mEntries)
    {
        ATProto::AppBskyActor::MutedWord mutedWord;

        if (entry.mRaw.startsWith('#'))
        {
            mutedWord.mValue = entry.mRaw.sliced(1);
            ATProto::AppBskyActor::MutedWord::Target tagTarget;
            tagTarget.mTarget = ATProto::AppBskyActor::MutedWordTarget::TAG;
            mutedWord.mTargets.push_back(tagTarget);
        }
        else
        {
            mutedWord.mValue = entry.mRaw;
            ATProto::AppBskyActor::MutedWord::Target tagTarget;
            tagTarget.mTarget = ATProto::AppBskyActor::MutedWordTarget::TAG;
            mutedWord.mTargets.push_back(tagTarget);
            ATProto::AppBskyActor::MutedWord::Target contentTarget;
            contentTarget.mTarget = ATProto::AppBskyActor::MutedWordTarget::CONTENT;
            mutedWord.mTargets.push_back(contentTarget);
        }

        mutedWordsPrefs.mItems.push_back(std::move(mutedWord));
    }
}

}
