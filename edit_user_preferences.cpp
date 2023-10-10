// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "edit_user_preferences.h"

namespace Skywalker {

EditUserPrefences::EditUserPrefences(QObject* parent) :
    QObject(parent)
{
}

const QString EditUserPrefences::getBirthDate() const
{
    if (!mBirthDate)
        return {};

    const auto date = mBirthDate->date();
    return date.toString(QLocale().dateFormat(QLocale::ShortFormat));
}

void EditUserPrefences::setUserPreferences(const ATProto::UserPreferences& userPreferences)
{
    mBirthDate = userPreferences.getBirthDate();
    mHomeFeedPref = userPreferences.getFeedViewPref("home");
}

void EditUserPrefences::setHideReplies(bool hide)
{
    if (hide != mHomeFeedPref.mHideReplies)
    {
        mHomeFeedPref.mHideReplies = hide;
        emit hideRepliesChanged();
    }
}

void EditUserPrefences::setHideRepliesByUnfollowed(bool hide)
{
    if (hide != mHomeFeedPref.mHideRepliesByUnfollowed)
    {
        mHomeFeedPref.mHideRepliesByUnfollowed = hide;
        emit hideRepliesByUnfollowedChanged();
    }
}

void EditUserPrefences::setHideReposts(bool hide)
{
    if (hide != mHomeFeedPref.mHideReposts)
    {
        mHomeFeedPref.mHideReposts = hide;
        emit hideRepostsChanged();
    }
}

void EditUserPrefences::setHideQuotePosts(bool hide)
{
    if (hide != mHomeFeedPref.mHideQuotePosts)
    {
        mHomeFeedPref.mHideQuotePosts = hide;
        emit hideQuotePostsChanged();
    }
}

}
