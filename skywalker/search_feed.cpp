// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "search_feed.h"
#include "unicode_fonts.h"

namespace Skywalker {

SearchFeed::SearchFeed(const QString& searchQuery, const SearchOptions& searchOptions) :
    mSearchQuery(searchQuery),
    mSearchOptions(searchOptions)
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
           mSearchOptions.equals(other.mSearchOptions);
}

const QString SearchFeed::getKey() const
{
    if (mSearchOptions.isDefault())
        return mSearchQuery;

    const auto optionsJson = toJson();
    const auto optionsBytes = QJsonDocument{optionsJson}.toJson(QJsonDocument::Compact);
    const auto hash = QCryptographicHash::hash(optionsBytes, QCryptographicHash::Md5);

    return QString("%1-%2").arg(mSearchQuery).arg(hash.toHex());
}

LanguageList SearchFeed::getLanguageList() const
{
    if (mSearchOptions.getLanguage().isEmpty())
        return {};

    return LanguageUtils::getLanguages(QStringList{mSearchOptions.getLanguage()});
}

QJsonObject SearchFeed::toJson() const
{
    QJsonObject json;
    json.insert("searchQuery", mSearchQuery);

    if (mSearchOptions.getExactPhrase())
        json.insert("exactPhrase", "true");

    if (mSearchOptions.getFollowing())
        json.insert("following", "true");

    if (!mSearchOptions.getAuthors().isEmpty())
        json.insert("authors", QJsonArray::fromStringList(mSearchOptions.getAuthors()));

    if (!mSearchOptions.getMentions().isEmpty())
        json.insert("mentions", QJsonArray::fromStringList(mSearchOptions.getMentions()));

    if (mSearchOptions.getSetSince() && mSearchOptions.getSince().isValid())
        json.insert("since", mSearchOptions.getSince().toUTC().toString(Qt::ISODateWithMs));

    if (mSearchOptions.getSetUntil() && mSearchOptions.getUntil().isValid())
        json.insert("until", mSearchOptions.getUntil().toUTC().toString(Qt::ISODateWithMs));

    if (!mSearchOptions.getLanguage().isEmpty())
        json.insert("language", mSearchOptions.getLanguage());

    if (mSearchOptions.getPostFilter() != SearchOptions::POST_FILTER_ALL)
        json.insert("postFilter", (int)mSearchOptions.getPostFilter());

    if (mSearchOptions.getMediaPostFilter() != SearchOptions::MEDIA_POST_FILTER_ALL)
        json.insert("mediaPostFilter", (int)mSearchOptions.getMediaPostFilter());

    if (!mSearchOptions.getExcludeWords().isEmpty())
        json.insert("excludeWords", QJsonArray::fromStringList(mSearchOptions.getExcludeWords()));

    return json;
}

SearchFeed SearchFeed::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    SearchFeed feed;
    feed.mSearchQuery = xjson.getRequiredString("searchQuery");
    feed.mSearchOptions.setFollowing(xjson.getOptionalBool("following", false));
    feed.mSearchOptions.setExactPhrase(xjson.getOptionalBool("exactPhrase", false));

    // Author used to be a single value, now it is a list
    auto author = xjson.getOptionalString("author");

    if (author)
        feed.mSearchOptions.setAuthors({*author,});
    else
        feed.mSearchOptions.setAuthors(xjson.getOptionalStringList("authors"));

    auto mention = xjson.getOptionalString("mention");

    if (mention)
        feed.mSearchOptions.setMentions({*mention,});
    else
        feed.mSearchOptions.setMentions(xjson.getOptionalStringList("mentions"));

    auto since = xjson.getOptionalDateTime("since");

    if (since)
    {
        feed.mSearchOptions.setSetSince(true);
        feed.mSearchOptions.setSince(*since);
    }

    auto until = xjson.getOptionalDateTime("until");

    if (until)
    {
        feed.mSearchOptions.setSetUntil(true);
        feed.mSearchOptions.setUntil(*until);
    }

    feed.mSearchOptions.setLanguage(xjson.getOptionalString("language", ""));

    const int postFilter = xjson.getOptionalInt("postFilter", 0);

    if (postFilter >= 0 && postFilter <= SearchOptions::POST_FILTER_LAST)
        feed.mSearchOptions.setPostFilter((SearchOptions::PostFilter)postFilter);

    const int mediaPostFilter = xjson.getOptionalInt("mediaPostFilter", 0);

    if (mediaPostFilter >= 0 && mediaPostFilter <= SearchOptions::MEDIA_POST_FILTER_LAST)
        feed.mSearchOptions.setMediaPostFilter((SearchOptions::MediaPostFilter)mediaPostFilter);

    feed.mSearchOptions.setExcludeWords(xjson.getOptionalStringList("excludeWords"));

    return feed;
}

}
