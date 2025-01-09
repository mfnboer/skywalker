// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "search_feed.h"
#include "unicode_fonts.h"

namespace Skywalker {

SearchFeed::SearchFeed(const QString& searchQuery, const QString& authorHandle, const QString& mentionsHandle,
               QDateTime since, QDateTime until, const QString& language) :
    mSearchQuery(searchQuery),
    mAuthorHandle(authorHandle),
    mMentionHandle(mentionsHandle),
    mSince(since),
    mUntil(until),
    mLanguage(language)
{
}

bool SearchFeed::isHashtag() const
{
    return UnicodeFonts::isHashtag(mSearchQuery);
}

bool SearchFeed::equals(const SearchFeed& other) const
{
    return mSearchQuery == other.mSearchQuery &&
        mAuthorHandle == other.mAuthorHandle &&
        mMentionHandle == other.mMentionHandle &&
        mSince == other.mSince &&
        mUntil == other.mUntil &&
        mLanguage == other.mLanguage;
}

LanguageList SearchFeed::getLanguageList() const
{
    if (mLanguage.isEmpty())
        return {};

    return LanguageUtils::getLanguages(QStringList{mLanguage});
}

QJsonObject SearchFeed::toJson() const
{
    QJsonObject json;
    json.insert("searchQuery", mSearchQuery);

    if (!mAuthorHandle.isEmpty())
        json.insert("author", mAuthorHandle);

    if (!mMentionHandle.isEmpty())
        json.insert("mention", mMentionHandle);

    if (mSince.isValid())
        json.insert("since", mSince.toString(Qt::ISODateWithMs));

    if (mUntil.isValid())
        json.insert("until", mUntil.toString(Qt::ISODateWithMs));

    if (!mLanguage.isEmpty())
        json.insert("language", mLanguage);

    return json;
}

SearchFeed SearchFeed::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    SearchFeed feed;
    feed.mSearchQuery = xjson.getRequiredString("searchQuery");
    feed.mAuthorHandle = xjson.getOptionalString("author", "");
    feed.mMentionHandle = xjson.getOptionalString("mention", "");
    feed.mSince = xjson.getOptionalDateTime("since", {});
    feed.mUntil = xjson.getOptionalDateTime("until", {});
    feed.mLanguage = xjson.getOptionalString("language", "");

    return feed;
}

}
