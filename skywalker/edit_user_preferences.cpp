// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "edit_user_preferences.h"
#include "definitions.h"

namespace Skywalker {

EditUserPreferences::EditUserPreferences(QObject* parent) :
    QObject(parent)
{
}

const QString EditUserPreferences::getBirthDate() const
{
    if (!mBirthDate)
        return {};

    const auto date = mBirthDate->date();
    return date.toString(QLocale().dateFormat(QLocale::ShortFormat));
}

void EditUserPreferences::setUserPreferences(const ATProto::UserPreferences& userPreferences)
{
    mBirthDate = userPreferences.getBirthDate();
    mHomeFeedPref = userPreferences.getFeedViewPref(HOME_FEED);
}

void EditUserPreferences::saveTo(ATProto::UserPreferences& userPreferences)
{
    userPreferences.setFeedViewPref(mHomeFeedPref);
}

void EditUserPreferences::setLoggedOutVisibility(bool visibility)
{
    if (visibility != mLoggedOutVisibility)
    {
        mLoggedOutVisibility = visibility;
        emit loggedOutVisibilityChanged();
    }
}

void EditUserPreferences::setHideReplies(bool hide)
{
    if (hide != mHomeFeedPref.mHideReplies)
    {
        mHomeFeedPref.mHideReplies = hide;
        mModified = true;
        emit hideRepliesChanged();
    }
}

void EditUserPreferences::setHideRepliesByUnfollowed(bool hide)
{
    if (hide != mHomeFeedPref.mHideRepliesByUnfollowed)
    {
        mHomeFeedPref.mHideRepliesByUnfollowed = hide;
        mModified = true;
        emit hideRepliesByUnfollowedChanged();
    }
}

void EditUserPreferences::setHideReposts(bool hide)
{
    if (hide != mHomeFeedPref.mHideReposts)
    {
        mHomeFeedPref.mHideReposts = hide;
        mModified = true;
        emit hideRepostsChanged();
    }
}

void EditUserPreferences::setHideQuotePosts(bool hide)
{
    if (hide != mHomeFeedPref.mHideQuotePosts)
    {
        mHomeFeedPref.mHideQuotePosts = hide;
        mModified = true;
        emit hideQuotePostsChanged();
    }
}

void EditUserPreferences::setShowQuotesWithBlockedPost(bool show)
{
    if (show != mShowQuotesWithBlockedPost)
    {
        mShowQuotesWithBlockedPost = show;
        setLocalSettingsModified(true);
        emit showQuotesWithBlockedPostChanged();
    }
}

void EditUserPreferences::setRewindToLastSeenPost(bool rewind)
{
    if (rewind != mRewindToLastSeenPost)
    {
        mRewindToLastSeenPost = rewind;
        setLocalSettingsModified(true);
        emit rewindToLastSeenPostChanged();
    }
}

void EditUserPreferences::setContentLanguages(const QStringList& contentLanguages)
{
    if (contentLanguages != mContentLanguages)
    {
        mContentLanguages = contentLanguages;
        setLocalSettingsModified(true);
        emit contentLanguagesChanged();
    }
}

void EditUserPreferences::setShowUnknownContentLanguage(bool show)
{
    if (show != mShowUnknownContentLanguage)
    {
        mShowUnknownContentLanguage = show;
        setLocalSettingsModified(true);
        emit showUnknownContentLanguageChanged();
    }
}

void EditUserPreferences::setDisplayMode(QEnums::DisplayMode displayMode)
{
    if (displayMode != mDisplayMode)
    {
        mDisplayMode = displayMode;
        setLocalSettingsModified(true);
        emit displayModeChanged();
    }
}

void EditUserPreferences::setGifAutoPlay(bool autoPlay)
{
    if (autoPlay != mGifAutoPlay)
    {
        mGifAutoPlay = autoPlay;
        setLocalSettingsModified(true);
        emit gifAutoPlayChanged();
    }
}

void EditUserPreferences::setNotificationsWifiOnly(bool wifiOnly)
{
    if (wifiOnly != mNotificationsWifiOnly)
    {
        mNotificationsWifiOnly = wifiOnly;
        setLocalSettingsModified(true);
        emit notificationsWifiOnlyChanged();
    }
}

}
