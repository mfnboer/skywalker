// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "post_filter.h"

namespace Skywalker {

FocusHashtagsPostFilter::FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry)
{
    const auto json = focusHashtaghEntry.toJson();
    auto* newEntry = FocusHashtagEntry::fromJson(json);
    mFocusHashtags.addEntry(newEntry);
}

QString FocusHashtagsPostFilter::getName() const
{
    const auto& entries = mFocusHashtags.getEntries();

    if (entries.empty())
    {
        qWarning() << "No entries";
        return "unknown";
    }

    const auto& hashtags = entries[0]->getHashtags();

    if (hashtags.empty())
    {
        qWarning() << "No hashtags";
        return "no hashtag";
    }

    return hashtags[0];
}

bool FocusHashtagsPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return mFocusHashtags.match(post);
}

AuthorPostFilter::AuthorPostFilter(const QString& did, const QString& handle) :
    mDid(did),
    mHandle(handle)
{
}

QString AuthorPostFilter::getName() const
{
    return mHandle;
}

bool AuthorPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return post.getAuthor().getDid() == mDid;
}

}
