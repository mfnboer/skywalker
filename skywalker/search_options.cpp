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

static QString getAuthorsString(const QStringList& authors)
{
    QString s;

    for (const auto& author : authors)
    {
        if (ATProto::ATRegex::isHandle(author))
            s += QString(" @%1").arg(author);
        else
            s += QString(" %1").arg(author);
    }

    return s;
}

QString SearchOptions::getDescription() const
{
    QStringList parts;

    if (mExactPrase)
        parts.push_back(QObject::tr("exact phrase"));

    if (mPostFilter == POST_FILTER_NO_REPLIES)
        parts.push_back(QObject::tr("no replies"));
    else if (mPostFilter == POST_FILTER_ONLY_REPLIES)
        parts.push_back(QObject::tr("only replies"));

    if (mMediaPostFilter == MEDIA_POST_FILTER_MEDIA)
        parts.push_back(QObject::tr("only media"));
    else if (mMediaPostFilter == MEDIA_POST_FILTER_VIDEO)
        parts.push_back(QObject::tr("only video"));

    if (mFollowing)
        parts.push_back(QObject::tr("from: users you follow"));

    if (!mAuthors.empty())
    {
        auto text = getAuthorsString(mAuthors);
        parts.push_back(QObject::tr("from:%1").arg(text));
    }

    if (!mMentions.empty())
    {
        auto text = getAuthorsString(mMentions);
        parts.push_back(QObject::tr("from:%1").arg(text));
    }

    if (!mExcludeWords.empty())
    {
        auto text = mExcludeWords.join(" ");
        parts.push_back(QObject::tr("exclude: %1").arg(text));
    }

    if (mSetSince)
    {
        auto text = mSince.toString(QLocale().dateFormat(QLocale::ShortFormat));
        parts.push_back(QObject::tr("since: %1").arg(text));
    }

    if (mSetUntil)
    {
        auto text = mUntil.toString(QLocale().dateFormat(QLocale::ShortFormat));
        parts.push_back(QObject::tr("until: %1").arg(text));
    }

    if (!mLanguage.isEmpty())
        parts.push_back(QObject::tr("language: %1").arg(mLanguage));

    return parts.join(", ");
}

bool SearchOptions::equals(const SearchOptions& other) const
{
    return mSortOrder == other.mSortOrder &&
           mExactPrase == other.mExactPrase &&
           mFollowing == other.mFollowing &&
           mAuthors == other.mAuthors &&
           mMentions == other.mMentions &&
           mSetSince == other.mSetSince &&
           (!mSetSince || mSince == other.mSince) &&
           mSetUntil == other.mSetUntil &&
           (!mSetUntil || mUntil == other.mUntil) &&
           mLanguage == other.mLanguage &&
           mPostFilter == other.mPostFilter &&
           mMediaPostFilter == other.mMediaPostFilter &&
           mExcludeWords == other.mExcludeWords;
}

bool SearchOptions::isDefault() const
{
    const SearchOptions defaultOptions;
    return equals(defaultOptions);
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
