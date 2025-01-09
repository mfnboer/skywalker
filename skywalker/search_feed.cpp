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

}
