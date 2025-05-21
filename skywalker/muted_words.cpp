// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "muted_words.h"
#include "search_utils.h"
#include "link_utils.h"

namespace Skywalker {

MutedWordEntry::MutedWordEntry(const QString& value, QEnums::ActorTarget actorTarget, QDateTime expiresAt) :
    mValue(value),
    mActorTarget(actorTarget),
    mExpiresAt(expiresAt)
{
    mIsDomain = LinkUtils::isDomain(mValue);
}

MutedWords::MutedWords(const ProfileStore& userFollows, QObject* parent) :
    QObject(parent),
    mUserFollows(userFollows)
{
}

MutedWordEntry::List MutedWords::getEntries() const
{
    MutedWordEntry::List sortedEntries;

    for (const auto& entry : mEntries)
        sortedEntries.append(MutedWordEntry(entry.mRaw, entry.mActorTarget, entry.mExpiresAt));

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
    mHashTagIndex.clear();
    mDomainIndex.clear();

    emit entriesChanged();
}

// A word can be a phrase..
static QString cleanRawWord(const QString& word)
{
    if (LinkUtils::isDomain(word))
        return word.toLower();

    const auto& words = SearchUtils::getWords(word);
    QString cleanedWord;

    if (UnicodeFonts::isHashtag(word) && words.size() == 1)
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

    if (UnicodeFonts::isHashtag(entry.mRaw))
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

bool MutedWords::Entry::isDomain() const
{
    return LinkUtils::isDomain(mRaw);
}

void MutedWords::addEntry(const QString& word, QEnums::ActorTarget actorTarget, QDateTime expiresAt)
{
    addEntry(word, {} , {}, actorTarget, expiresAt);
}

void MutedWords::addEntry(const QString& word, const QJsonObject& bskyJson, const QStringList& unkwownTargets,
                          QEnums::ActorTarget actorTarget, QDateTime expiresAt)
{
    Entry newEntry(cleanRawWord(word), SearchUtils::getNormalizedWords(word), bskyJson, unkwownTargets);
    newEntry.mActorTarget = actorTarget;
    newEntry.mExpiresAt = expiresAt;

    if (newEntry.isDomain())
    {
        if (newEntry.mRaw.startsWith('.'))
        {
            if (newEntry.mRaw.count('.') > 1)
            {
                newEntry.mRaw.slice(1);
                newEntry.mNormalizedWords = { newEntry.mRaw };
            }
            else
            {
                newEntry.mNormalizedWords = { newEntry.mRaw.sliced(1) };
            }
        }
        else
        {
            newEntry.mNormalizedWords = { newEntry.mRaw };
        }

        qDebug() << "Domain entry:" << newEntry.mRaw;
    }

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
    else if (entry.isDomain())
        addWordToIndex(&entry, mDomainIndex);
    else if (entry.mNormalizedWords.size() == 1)
        addWordToIndex(&entry, mSingleWordIndex);
    else if (entry.mNormalizedWords.size() > 1)
        addWordToIndex(&entry, mFirstWordIndex);

    mDirty = true;
    emit entriesChanged();
}

void MutedWords::removeEntry(const QString& word)
{
    const Entry searchEntry{ word, {}, {}, {} };
    const auto it = mEntries.find(searchEntry);

    if (it == mEntries.end())
    {
        qDebug() << "Entry not found:" << word;
        return;
    }

    const Entry& entry = *it;

    if (entry.isHashtag())
        removeWordFromIndex(&entry, mHashTagIndex);
    else if (entry.isDomain())
        removeWordFromIndex(&entry, mDomainIndex);
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
    const Entry searchEntry{ word, {}, {}, {} };
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

bool MutedWords::mustSkip(const Entry& entry, const QString& authorDid, QDateTime now) const
{
    if (entry.mExpiresAt.isValid() && now >= entry.mExpiresAt)
    {
        qDebug() << "Expired entry:" << entry.mRaw << entry.mExpiresAt;
        return true;
    }

    if (entry.mActorTarget == QEnums::ACTOR_TARGET_EXCLUDE_FOLLOWING &&
        !authorDid.isEmpty() && mUserFollows.contains(authorDid))
    {
        qDebug() << "Entry from follower:" << entry.mRaw << authorDid;
        return true;
    }

    return false;
}

bool MutedWords::match(const NormalizedWordIndex& post) const
{
    if (mEntries.empty())
        return false;

    const auto now = QDateTime::currentDateTimeUtc();
    const QString authorDid = post.getAuthorDid();

    if (matchDomain(post, now, authorDid))
        return true;

    if (matchHashtag(post, now, authorDid))
        return true;

    return matchWords(post, now, authorDid);
}

bool MutedWords::matchDomain(const NormalizedWordIndex& post, QDateTime now, const QString& authorDid) const
{
    if (mDomainIndex.empty())
        return false;

    const auto& domains = post.getUniqueDomains();

    for (const auto& [word, entries] : mDomainIndex)
    {
        Q_ASSERT(entries.size() == 1); // TODO: fires when switching from skywalker to skywalkertest
        const auto* entry = *entries.begin();

        if (mustSkip(*entry, authorDid, now))
            continue;

        // NOTE: the number of domains should be small, typically 1
        for (const auto& domain : domains)
        {
            if (domain == word)
            {
                qDebug() << "Match on domain:" << word;
                return true;
            }
            else if (domain.endsWith(word) && domain.at(domain.length() - word.length() - 1) == '.')
            {
                qDebug() << "Match on domain:" << word;
                return true;
            }
        }
    }

    return false;
}

bool MutedWords::matchHashtag(const NormalizedWordIndex& post, QDateTime now, const QString& authorDid) const
{
    if (mHashTagIndex.empty())
        return false;

    const auto& postHashtags = post.getUniqueHashtags();

    for (const auto& [word, entries] : mHashTagIndex)
    {
        Q_ASSERT(entries.size() == 1);
        const auto* entry = *entries.begin();

        if (!mustSkip(*entry, authorDid, now) && postHashtags.count(word))
        {
            qDebug() << "Match on hashtag:" << word;
            return true;
        }
    }

    return false;
}

bool MutedWords::matchWords(const NormalizedWordIndex& post, QDateTime now, const QString& authorDid) const
{
    const auto& uniquePostWords = post.getUniqueNormalizedWords();

    for (const auto& [word, entries] : mSingleWordIndex)
    {
        Q_ASSERT(entries.size() == 1);
        const auto* entry = *entries.begin();

        if (!mustSkip(*entry, authorDid, now) && uniquePostWords.count(word))
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

            if (mustSkip(*mutedEntry, authorDid, now))
                continue;

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
    qDebug() << "Load muted words";
    clear();
    const auto& mutedWords = userPrefs.getMutedWordsPref();

    for (const auto& mutedWord : mutedWords.mItems)
    {
        bool matchContent = false;
        bool matchTag = false;
        QStringList unknownTargets;

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
                unknownTargets.push_back(target.mRawTarget);
                break;
            }
        }

        if  (matchTag && !matchContent)
        {
            addEntry(QString("#%1").arg(mutedWord.mValue), mutedWord.mJson, unknownTargets,
                     (QEnums::ActorTarget)mutedWord.mActorTarget,
                     mutedWord.mExpiresAt.value_or(QDateTime{}));
        }
        else
        {
            addEntry(mutedWord.mValue, mutedWord.mJson, unknownTargets,
                     (QEnums::ActorTarget)mutedWord.mActorTarget,
                     mutedWord.mExpiresAt.value_or(QDateTime{}));
        }
    }

    qDebug() << "Muted words loaded:" << mEntries.size();
    mDirty = false;
}

static ATProto::AppBskyActor::MutedWord::Target makeTarget(
    const ATProto::AppBskyActor::MutedWordTarget& targetType, const QString& rawType = {})
{
    ATProto::AppBskyActor::MutedWord::Target target;
    target.mTarget = targetType;

    if (rawType.isEmpty())
        target.mRawTarget = ATProto::AppBskyActor::mutedWordTargetToString(targetType);
    else
        target.mRawTarget = rawType;

    return target;
}

void MutedWords::save(ATProto::UserPreferences& userPrefs)
{
    if (!mDirty)
        return;

    ATProto::UserPreferences::MutedWordsPref& mutedWordsPrefs = userPrefs.getMutedWordsPref();
    mutedWordsPrefs.mItems.clear();

    for (const auto& entry : mEntries)
    {
        ATProto::AppBskyActor::MutedWord mutedWord;

        if (UnicodeFonts::isHashtag(entry.mRaw))
        {
            mutedWord.mValue = entry.mRaw.sliced(1);
            mutedWord.mTargets.push_back(makeTarget(ATProto::AppBskyActor::MutedWordTarget::TAG));
        }
        else
        {
            mutedWord.mValue = entry.mRaw;
            mutedWord.mTargets.push_back(makeTarget(ATProto::AppBskyActor::MutedWordTarget::TAG));
            mutedWord.mTargets.push_back(makeTarget(ATProto::AppBskyActor::MutedWordTarget::CONTENT));
        }

        if (entry.mExpiresAt.isValid())
            mutedWord.mExpiresAt = entry.mExpiresAt;

        mutedWord.mActorTarget = (ATProto::AppBskyActor::ActorTarget)entry.mActorTarget;

        // Forward compatibility: save unknown targets
        for (const auto& unknownTarget : entry.mUnknownTargets)
        {
            const auto target = makeTarget(ATProto::AppBskyActor::MutedWordTarget::UNKNOWN, unknownTarget);
            mutedWord.mTargets.push_back(target);
        }

        // Forward compatibility: save full Bsky json.
        mutedWord.mJson = entry.mBskyJson;

        mutedWordsPrefs.mItems.push_back(std::move(mutedWord));
    }
}

}
