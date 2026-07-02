// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_notification_utils.h"
#include "skywalker.h"

namespace Skywalker {

ChatNotificationUtils::ChatNotificationUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void ChatNotificationUtils::getChatNotificationPrefs()
{
    if (!bskyClient())
        return;

    bskyClient()->getChatNotificationPreferences(
        [this, presence=getPresence()](auto prefs){
            if (!presence)
                return;

            mEditChatNotificationPrefs = std::make_unique<EditChatNotificationPreferences>(prefs->mPreferences);
            emit chatNotificationPrefsOk(mEditChatNotificationPrefs.get());
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getChatNotificationPreferences failed:" << error << " - " << msg;
            emit chatNotificationPrefsFailed(msg);
        });
}

void ChatNotificationUtils::saveChatNotificationPrefs()
{
    qDebug() << "Save notification preferences";

    if (!mEditChatNotificationPrefs || mEditChatNotificationPrefs->isNull())
    {
        qDebug() << "No preferences loaded.";
        return;
    }

    if (!arePreferencesModified())
    {
        qDebug() << "No modified preferences.";
        return;
    }

    ATProto::ChatBskyNotification::ChatPreference::SharedPtr chatPref =
        mEditChatNotificationPrefs->isChatModified() ? mEditChatNotificationPrefs->getPrefs()->mChat : nullptr;
    ATProto::ChatBskyNotification::ChatPreference::SharedPtr chatRequestPref =
        mEditChatNotificationPrefs->isChatRequestModified() ? mEditChatNotificationPrefs->getPrefs()->mChatRequest : nullptr;

    bskyClient()->putChatNotificationPreferences(chatPref, chatRequestPref,
        [presence=getPresence()](auto){
            if (!presence)
                return;

            qDebug() << "Notifications preferences saved.";
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "putChatNotificationPreferences failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Failed to save chat notification settings"), QEnums::STATUS_LEVEL_ERROR);
        });
}

bool ChatNotificationUtils::arePreferencesModified() const
{
    if (!mEditChatNotificationPrefs)
        return false;

    return mEditChatNotificationPrefs->isChatModified() ||
           mEditChatNotificationPrefs->isChatRequestModified();
}

}
