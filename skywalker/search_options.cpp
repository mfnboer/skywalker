// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "search_options.h"
#include <atproto/lib/at_regex.h>

namespace Skywalker {

static constexpr char const* USER_ME = "me";

ATProto::Client::SearchParams SearchOptions::createSearchParams(const QString& userDid) const
{
    QStringList authorList = cleanHandleList(mAuthors, userDid);
    QStringList mentionList = cleanHandleList(mMentions, userDid);

    ATProto::Client::SearchParams searchParams;
    searchParams.setSort(mSortOrder);

    if (mFollowing)
        searchParams.setFollowing(mFollowing);

    if (!authorList.isEmpty())
        searchParams.addAuthors(authorList);

    if (!mentionList.isEmpty())
        searchParams.addMentions(mentionList);

    if (mSetSince)
        searchParams.setSince(mSince.toUTC());

    if (mSetUntil)
        searchParams.setUntil(mUntil.toUTC());

    if (!mLanguage.isEmpty())
        searchParams.addLanguages({mLanguage,});

    switch (mPostFilter)
    {
    case POST_FILTER_ALL:
        break;
    case POST_FILTER_NO_REPLIES:
        searchParams.setExcludeReplies(true);
        break;
    case POST_FILTER_ONLY_REPLIES:
        searchParams.setRepliesOnly(true);
        break;
    }

    switch (mMediaPostFilter)
    {
    case MEDIA_POST_FILTER_ALL:
        break;
    case MEDIA_POST_FILTER_MEDIA:
        searchParams.setHasMedia(true);
        break;
    case MEDIA_POST_FILTER_VIDEO:
        searchParams.setHasVideo(true);
        break;
    }

    // It is not clear what the recency window is. Search the whole index.
    searchParams.setAllTime(true);
    return searchParams;
}

QStringList SearchOptions::validateHandles(const QStringList& handles)
{
    std::set<QString> handleSet;

    for (const auto& handle : handles)
    {
        if (handle == USER_ME)
            handleSet.insert(handle);
        else if (ATProto::ATRegex::isHandle(handle))
            handleSet.insert(handle);
        else
            qWarning() << "Invalid handle:" << handle;
    }

    return QStringList{handleSet.begin(), handleSet.end()};
}

QStringList SearchOptions::cleanHandleList(const QStringList& authors, const QString& userDid)
{
    QStringList handleList = validateHandles(authors);

    for (auto& handle : handleList)
    {
        if (handle == USER_ME)
            handle = userDid;
    }

    return handleList;
}


}
