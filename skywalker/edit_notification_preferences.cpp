// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "edit_notification_preferences.h"

namespace Skywalker {

EditNotificationPreferences::EditNotificationPreferences(QObject* parent) :
    QObject(parent)
{}

EditNotificationPreferences::EditNotificationPreferences(const ATProto::AppBskyNotification::Preferences::SharedPtr& prefs, QObject* parent) :
    QObject(parent),
    mPrefs(prefs),
    mFollowPref(new EditNotificationFilterablePref(mPrefs->mFollow, this)),
    mLikePref(new EditNotificationFilterablePref(mPrefs->mLike, this)),
    mLikeViaRepostPref(new EditNotificationFilterablePref(mPrefs->mLikeViaRepost, this)),
    mMentionPref(new EditNotificationFilterablePref(mPrefs->mMention, this)),
    mQuotePref(new EditNotificationFilterablePref(mPrefs->mQuote, this)),
    mReplyPref(new EditNotificationFilterablePref(mPrefs->mReply, this)),
    mRepostPref(new EditNotificationFilterablePref(mPrefs->mRepost, this)),
    mRepostViaRepostPref(new EditNotificationFilterablePref(mPrefs->mRepostViaRepost, this)),
    mSubscribedPostPref(new EditNotificationPref(mPrefs->mSubscribedPost, this))
{
}

void EditNotificationPreferences::setChatIncludeType(QEnums::NotifcationChatIncludeType includeType)
{
    if (includeType != getChatIncludeType())
    {
        mPrefs->mChat->mInclude = (ATProto::AppBskyNotification::ChatPreference::IncludeType)includeType;
        mPrefs->mChat->mRawInclude = ATProto::AppBskyNotification::ChatPreference::includeTypeToString(mPrefs->mChat->mInclude, "");
        mChatModified = true;
        emit chatIncludeTypeChanged();
    }
}

void EditNotificationPreferences::setChatPush(bool push)
{
    if (push != getChatPush())
    {
        mPrefs->mChat->mPush = push;
        emit chatPushChanged();
    }
}

bool EditNotificationPreferences::isModified() const
{
    return isChatModified() ||
        isFollowModified() ||
        isLikeModified() ||
        isLikeViaRepostModified() ||
        isMentionModified() ||
        isQuoteModified() ||
        isReplyModified() ||
        isRepostModified() ||
        isRepostViaRepostModified() ||
        isSubscribedPostModified();
}

}
