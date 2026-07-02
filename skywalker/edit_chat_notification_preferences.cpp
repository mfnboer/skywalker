// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "edit_chat_notification_preferences.h"

namespace Skywalker {

EditChatNotificationPreferences::EditChatNotificationPreferences(QObject* parent) :
    QObject(parent)
{
}

EditChatNotificationPreferences::EditChatNotificationPreferences(const ATProto::ChatBskyNotification::Preferences::SharedPtr& prefs,
                                QObject* parent) :
    QObject(parent),
    mPrefs(prefs),
    mChatPref(new EditChatNotificationPref(mPrefs->mChat, this)),
    mChatRequestPref(new EditChatNotificationPref(mPrefs->mChatRequest, this))
{
}

}
