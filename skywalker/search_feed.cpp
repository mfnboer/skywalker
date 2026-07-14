// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "search_feed.h"
#include "unicode_fonts.h"

namespace Skywalker {

SearchFeed::SearchFeed(const QString& searchQuery, bool following,
                       const QStringList& authorHandles, const QStringList& mentionHandles,
                       QDateTime since, QDateTime until, const QString& language) :
    mSearchQuery(searchQuery),
    mFollowing(following),
    mAuthorHandles(authorHandles),
    mMentionHandles(mentionHandles),
    mSince(since),
    mUntil(until),
    mLanguage(language)
{
}

bool SearchFeed::isHashtag() const
{
    return UnicodeFonts::isHashtag(mSearchQuery);
}

bool SearchFeed::isCashtag() const
{
    return UnicodeFonts::isCashtag(mSearchQuery);
}

bool SearchFeed::equals(const SearchFeed& other) const
{
    return mSearchQuery == other.mSearchQuery &&
        mFollowing == other.mFollowing &&
        mAuthorHandles == other.mAuthorHandles &&
        mMentionHandles == other.mMentionHandles &&
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

    if (mFollowing)
        json.insert("following", "true");

    if (!mAuthorHandles.isEmpty())
        json.insert("authors", QJsonArray::fromStringList(mAuthorHandles));

    if (!mMentionHandles.isEmpty())
        json.insert("mentions", QJsonArray::fromStringList(mMentionHandles));

    if (mSince.isValid())
        json.insert("since", mSince.toUTC().toString(Qt::ISODateWithMs));

    if (mUntil.isValid())
        json.insert("until", mUntil.toUTC().toString(Qt::ISODateWithMs));

    if (!mLanguage.isEmpty())
        json.insert("language", mLanguage);

    return json;
}

SearchFeed SearchFeed::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    SearchFeed feed;
    feed.mSearchQuery = xjson.getRequiredString("searchQuery");
    feed.mFollowing = xjson.getOptionalBool("following", false);

    // Author used to be a single value, now it is a list
    auto author = xjson.getOptionalString("author");

    if (author)
        feed.mAuthorHandles.push_back(*author);
    else
        feed.mAuthorHandles = xjson.getOptionalStringList("authors");

    auto mention = xjson.getOptionalString("mention");

    if (mention)
        feed.mMentionHandles.push_back(*mention);
    else
        feed.mMentionHandles = xjson.getOptionalStringList("mentions");

    feed.mSince = xjson.getOptionalDateTime("since", {});
    feed.mUntil = xjson.getOptionalDateTime("until", {});
    feed.mLanguage = xjson.getOptionalString("language", "");

    return feed;
}

}
