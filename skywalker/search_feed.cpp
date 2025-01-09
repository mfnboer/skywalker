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

}
