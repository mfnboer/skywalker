// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "focus_hashtags.h"
#include "search_utils.h"
#include "user_settings.h"
#include <atproto/lib/xjson.h>
#include <QJsonArray>
#include <QDebug>

namespace Skywalker {

int FocusHashtagEntry::sNextId = 1;

FocusHashtagEntry::FocusHashtagEntry(QObject* parent) :
    QObject(parent),
    mId{sNextId++}
{
}

FocusHashtagEntry* FocusHashtagEntry::fromJson(const QJsonObject& json, QObject* parent)
{
    auto* entry = new FocusHashtagEntry(parent);
    ATProto::XJsonObject xjson(json);
    const auto& hashtags = xjson.getRequiredStringVector("hashtags");
    entry->mHashtags.insert(hashtags.begin(), hashtags.end());
    entry->mHighlightColor = xjson.getRequiredString("highlightColor");
    return entry;
}

QJsonObject FocusHashtagEntry::toJson() const
{
    QJsonObject json;
    QJsonArray hashtags;

    for (const auto& tag : mHashtags)
        hashtags.append(tag);

    json.insert("hashtags", hashtags);
    json.insert("highlightColor", mHighlightColor.name());
    return json;
}

QStringList FocusHashtagEntry::getHashtags() const
{
    return QStringList(mHashtags.begin(), mHashtags.end());
}

bool FocusHashtagEntry::addHashtag(const QString& hashtag)
{
    if (mHashtags.size() >= MAX_HASHTAGS)
    {
        qWarning() << "Cannot add hashtag, size:" << mHashtags.size();
        return false;
    }

    if (!mHashtags.contains(SearchUtils::normalizeText(hashtag)))
    {
        mHashtags.insert(hashtag);
        emit hashtagsChanged();
        return true;
    }

    return false;
}

bool FocusHashtagEntry::removeHashtag(const QString& hashtag)
{
    if (mHashtags.erase(hashtag))
    {
        emit hashtagsChanged();
        return true;
    }

    return false;
}

void FocusHashtagEntry::setHighlightColor(const QColor& color)
{
    if (color != mHighlightColor)
    {
        mHighlightColor = color;
        emit highlightColorChanged();
    }
}

FocusHashtags::FocusHashtags(QObject* parent) :
    QObject(parent)
{
}

QJsonDocument FocusHashtags::toJson() const
{
    QJsonArray entryArray;

    for (const auto* entry : mEntries)
        entryArray.append(entry->toJson());

    return QJsonDocument(entryArray);
}

void FocusHashtags::setEntries(const QJsonDocument& json)
{
    clear();

    if (json.isEmpty())
        return;

    const QJsonArray entryArray = json.array();

    for (const auto entryJson : entryArray)
    {
        auto* entry = FocusHashtagEntry::fromJson(entryJson.toObject(), this);
        addEntry(entry);
    }
}

void FocusHashtags::clear()
{
    mAllHashtags.clear();

    if (!mEntries.empty())
    {
        for (auto* entry : mEntries)
            delete entry;

        mEntries.clear();
        emit entriesChanged();
    }
}

void FocusHashtags::addEntry(FocusHashtagEntry* entry)
{
    if (mEntries.size() >= MAX_ENTRIES)
    {
        qWarning() << "Cannot add entry, size:" << mEntries.size();
        return;
    }

    const auto& hashtags = entry->getHashtags();

    for (const auto& tag : hashtags)
    {
        const QString normalizedTag = SearchUtils::normalizeText(tag);
        mAllHashtags[normalizedTag].insert(entry);
    }

    for (auto it = mEntries.cbegin(); it != mEntries.cend(); ++it)
    {
        if ((*it)->getHashtagSet() > entry->getHashtagSet())
        {
            mEntries.insert(it, entry);
            emit entriesChanged();
            return;
        }
    }

    mEntries.push_back(entry);
    emit entriesChanged();
}

void FocusHashtags::addEntry(const QString& hashtag, QColor highlightColor)
{
    if (mAllHashtags.contains(hashtag))
        return;

    auto* entry = new FocusHashtagEntry(this);
    entry->addHashtag(hashtag);

    if (highlightColor.isValid())
        entry->setHighlightColor(highlightColor);

    addEntry(entry);
}

void FocusHashtags::removeEntry(int entryId)
{
    for (int i = 0; i < mEntries.size(); ++i)
    {
        auto* entry = mEntries[i];

        if (entry->getId() == entryId)
        {
            const auto& hashtags = entry->getHashtags();

            for (const auto& tag : hashtags)
            {
                const QString normalizedTag = SearchUtils::normalizeText(tag);
                mAllHashtags[normalizedTag].erase(entry);

                if (mAllHashtags[normalizedTag].empty())
                    mAllHashtags.erase(normalizedTag);
            }

            mEntries.remove(i);
            delete entry;
            emit entriesChanged();

            break;
        }
    }
}

void FocusHashtags::addHashtagToEntry(FocusHashtagEntry* entry, const QString hashtag)
{
    if (entry->addHashtag(hashtag))
    {
        const QString normalizedTag = SearchUtils::normalizeText(hashtag);
        mAllHashtags[normalizedTag].insert(entry);
    }
}

void FocusHashtags::removeHashtagFromEntry(FocusHashtagEntry* entry, const QString hashtag)
{
    if (entry->removeHashtag(hashtag))
    {
        const QString normalizedTag = SearchUtils::normalizeText(hashtag);
        mAllHashtags[normalizedTag].erase(entry);

        if (mAllHashtags[normalizedTag].empty())
            mAllHashtags.erase(normalizedTag);

        if (entry->empty())
            removeEntry(entry->getId());
    }
}

std::pair<bool, const IMatchEntry*> FocusHashtags::match(const NormalizedWordIndex& post) const
{
    const std::vector<QString> tags = post.getAllTags();

    for (const auto& tag : tags)
    {
        const QString normalizedTag = SearchUtils::normalizeText(tag);

        if (mAllHashtags.contains(normalizedTag))
            return { true, nullptr };
    }

    return { false, nullptr };
}

QColor FocusHashtags::highlightColor(const NormalizedWordIndex& post) const
{
    const std::vector<QString> tags = post.getAllTags();

    for (const auto& tag : tags)
    {
        const QString normalizedTag = SearchUtils::normalizeText(tag);
        auto it = mAllHashtags.find(normalizedTag);

        if (it == mAllHashtags.end())
            continue;

        const std::unordered_set<FocusHashtagEntry*>& entries = it->second;

        if (!entries.empty())
        {
            const FocusHashtagEntry* entry = *entries.begin();
            return entry->getHightlightColor();
        }
    }

    return {};
}

FocusHashtagEntryList FocusHashtags::getMatchEntries(const NormalizedWordIndex& post) const
{
    std::unordered_set<FocusHashtagEntry*> matchEntries;
    const std::vector<QString> tags = post.getAllTags();

    for (const auto& tag : tags)
    {
        const QString normalizedTag = SearchUtils::normalizeText(tag);
        auto it = mAllHashtags.find(normalizedTag);

        if (it == mAllHashtags.end())
            continue;

        const std::unordered_set<FocusHashtagEntry*>& entries = it->second;
        matchEntries.insert(entries.begin(), entries.end());
    }

    return FocusHashtagEntryList(matchEntries.begin(), matchEntries.end());
}

std::set<QString> FocusHashtags::getNormalizedMatchHashtags(const NormalizedWordIndex& post) const
{
    std::set<QString> matchHashtags;
    const FocusHashtagEntryList entries = getMatchEntries(post);

    for (auto* entry : entries)
    {
        const auto& hashtags = entry->getHashtagSet();

        for (const auto& tag : hashtags)
        {
            const QString normalizedTag = SearchUtils::normalizeText(tag);
            matchHashtags.insert(normalizedTag);
        }
    }

    return matchHashtags;
}

void FocusHashtags::save(const QString& did, UserSettings* settings) const
{
    Q_ASSERT(settings);
    qDebug() << "Save focus hashtags:" << did;
    settings->setFocusHashtags(did, toJson());
}

void FocusHashtags::load(const QString& did, const UserSettings* settings)
{
    Q_ASSERT(settings);
    qDebug() << "Load focus hashtags:" << did;

    try {
        const auto json = settings->getFocusHashtags(did);
        setEntries(json);
    }
    catch (ATProto::InvalidJsonException& e) {
        qWarning() << "Could not load focus hashtags:" << e.msg();
    }
}

}
