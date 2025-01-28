// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "post_filter.h"

namespace Skywalker {

HashtagPostFilter::HashtagPostFilter(const QString& hashtag)
{
    if (hashtag.startsWith('#'))
        mFocusHashtags.addEntry(hashtag.sliced(1));
    else
        mFocusHashtags.addEntry(hashtag);
}

QString HashtagPostFilter::getName() const
{
    return "#" + mFocusHashtags.getEntries().first()->getHashtags().first();
}

bool HashtagPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return mFocusHashtags.match(post);
}

FocusHashtagsPostFilter::FocusHashtagsPostFilter(const FocusHashtagEntry& focusHashtaghEntry)
{
    const auto json = focusHashtaghEntry.toJson();
    auto* newEntry = FocusHashtagEntry::fromJson(json);
    mFocusHashtags.addEntry(newEntry);
}

const FocusHashtagEntry* FocusHashtagsPostFilter::getFocusHashtagEntry() const
{
    const auto& entries = mFocusHashtags.getEntries();

    if (entries.empty())
    {
        qWarning() << "No entries";
        return nullptr;
    }

    return entries[0];
}

QString FocusHashtagsPostFilter::getName() const
{
    const auto* entry = getFocusHashtagEntry();

    if (!entry)
        return "unknown";

    const auto& hashtags = entry->getHashtags();

    if (hashtags.empty())
    {
        qWarning() << "No hashtags";
        return "no hashtag";
    }

    QString name;

    for (const auto& tag : hashtags)
    {
        if (!name.isEmpty())
            name += ' ';

        name += "#" + tag;
    }

    return name;
}

QColor FocusHashtagsPostFilter::getBackgroundColor() const
{
    const auto* entry = getFocusHashtagEntry();

    if (!entry)
        return IPostFilter::getBackgroundColor();

    return entry->getHightlightColor();
}

bool FocusHashtagsPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return mFocusHashtags.match(post);
}

AuthorPostFilter::AuthorPostFilter(const BasicProfile& profile) :
    mProfile(profile)
{
}

QString AuthorPostFilter::getName() const
{
    return "@" + mProfile.getHandle();
}

BasicProfile AuthorPostFilter::getAuthor() const
{
    return mProfile;
}

bool AuthorPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;

    return post.getAuthor().getDid() == mProfile.getDid();
}

QString VideoPostFilter::getName() const
{
    return QObject::tr("Video");
}

bool VideoPostFilter::match(const Post& post) const
{
    if (post.isPlaceHolder())
        return false;


    return post.getVideoView() != nullptr;
}

}
