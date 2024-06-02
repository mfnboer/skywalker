// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "focus_hashtags.h"
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
    entry->mHashtags.assign(hashtags.begin(), hashtags.end());
    entry->mHighlightColorLightMode = xjson.getRequiredString("highlightColorLightMode");
    entry->mHighlightColorDarkMode = xjson.getRequiredString("highlightColorDarkMode");
    return entry;
}

QJsonObject FocusHashtagEntry::toJson() const
{
    QJsonObject json;
    QJsonArray hashtags;

    for (const auto& tag : mHashtags)
        hashtags.append(tag);

    json.insert("hashtags", hashtags);
    json.insert("highlightColorLightMode", mHighlightColorLightMode.name());
    json.insert("highlightColorDarkMode", mHighlightColorLightMode.name());
    return json;
}

void FocusHashtagEntry::addHashtag(const QString& hashtag)
{
    if (mHashtags.size() >= MAX_HASHTAGS)
    {
        qWarning() << "Cannot add hashtag, size:" << mHashtags.size();
        return;
    }

    if (!mHashtags.contains(hashtag))
    {
        mHashtags.push_back(hashtag);
        emit hashtagsChanged();
    }
}

void FocusHashtagEntry::removeHashtag(const QString& hashtag)
{
    if (mHashtags.removeOne(hashtag))
        emit hashtagsChanged();
}

const QColor& FocusHashtagEntry::getHightlightColor() const
{
    const auto displayMode = UserSettings::getActiveDisplayMode();

    switch (displayMode)
    {
    case QEnums::DISPLAY_MODE_LIGHT:
        return mHighlightColorLightMode;
    case QEnums::DISPLAY_MODE_DARK:
        return mHighlightColorDarkMode;
    case QEnums::DISPLAY_MODE_SYSTEM:
        Q_ASSERT(false);
        qWarning() << "Unexpected SYSTEM mode";
        return mHighlightColorLightMode;
    }

    Q_ASSERT(false);
    qWarning() << "Invalid display mode:" << displayMode;
    return mHighlightColorLightMode;
}

void FocusHashtagEntry::setHighlightColor(const QColor& color)
{
    if (color == getHightlightColor())
        return;

    const auto displayMode = UserSettings::getActiveDisplayMode();

    switch (displayMode)
    {
    case QEnums::DISPLAY_MODE_LIGHT:
        mHighlightColorLightMode = color;
        break;
    case QEnums::DISPLAY_MODE_DARK:
        mHighlightColorDarkMode = color;
        break;
    case QEnums::DISPLAY_MODE_SYSTEM:
        Q_ASSERT(false);
        qWarning() << "Unexpected SYSTEM mode";
        return;
    }

    emit highlightColorChanged();
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

    QJsonArray entryArray = json.array();

    for (const auto entryJson : entryArray)
    {
        auto* entry = FocusHashtagEntry::fromJson(entryJson.toObject());
        addEntry(entry);
    }
}

void FocusHashtags::clear()
{
    mAllHashtags.clear();

    if (!mEntries.empty())
    {
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
        ++mAllHashtags[tag];

    mEntries.push_back(entry);
    emit entriesChanged();
}

void FocusHashtags::addEntry(const QString& hashtag)
{
    auto* entry = new FocusHashtagEntry(this);
    entry->addHashtag(hashtag);
    addEntry(entry);
}

void FocusHashtags::removeEntry(int entryId)
{
    for (int i = 0; i < mEntries.size(); ++i)
    {
        const auto* entry = mEntries[i];

        if (entry->getId() == entryId)
        {
            const auto& hashtags = entry->getHashtags();

            for (const auto& tag : hashtags)
            {
                if (--mAllHashtags[tag] <= 0)
                    mAllHashtags.erase(tag);
            }

            mEntries.remove(i);
            delete entry;
            emit entriesChanged();

            break;
        }
    }
}

bool FocusHashtags::match(const NormalizedWordIndex& post) const
{
    const std::vector<QString> hashtags = post.getHashtags();

    for (const auto& tag : hashtags)
    {
        if (mAllHashtags.contains(tag))
            return true;
    }

    return false;
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
