// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "notification_utils.h"
#include "skywalker.h"

namespace Skywalker {

NotificationUtils::NotificationUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void NotificationUtils::getNotificationPrefs()
{
    if (!bskyClient())
        return;

    bskyClient()->getNotificationPreferences(
        [this, presence=getPresence()](auto prefs){
            if (!presence)
                return;

            mNotificationPrefs = std::make_unique<EditNotificationPreferences>(prefs->mPreferences);
            emit notificationPrefsOk(mNotificationPrefs.get());
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getNotificationPreferences failed:" << error << " - " << msg;
            emit notificationPrefsFailed(msg);
        });
}

void NotificationUtils::saveNotificationPrefs()
{
    qDebug() << "Save notification preferences";

    if (!mNotificationPrefs || mNotificationPrefs->isNull())
    {
        qDebug() << "No preferences loaded.";
        return;
    }

    if (!mNotificationPrefs->isModified())
    {
        qDebug() << "No modified preferences.";
        return;
    }

    ATProto::AppBskyNotification::Preferences prefs;

    if (mNotificationPrefs->isChatModified())
        prefs.mChat = mNotificationPrefs->getPrefs()->mChat;
    if (mNotificationPrefs->isFollowModified())
        prefs.mFollow = mNotificationPrefs->getPrefs()->mFollow;
    if (mNotificationPrefs->isLikeModified())
        prefs.mLike = mNotificationPrefs->getPrefs()->mLike;
    if (mNotificationPrefs->isLikeViaRepostModified())
        prefs.mLikeViaRepost = mNotificationPrefs->getPrefs()->mLikeViaRepost;
    if (mNotificationPrefs->isMentionModified())
        prefs.mMention = mNotificationPrefs->getPrefs()->mMention;
    if (mNotificationPrefs->isQuoteModified())
        prefs.mQuote = mNotificationPrefs->getPrefs()->mQuote;
    if (mNotificationPrefs->isReplyModified())
        prefs.mReply = mNotificationPrefs->getPrefs()->mReply;
    if (mNotificationPrefs->isRepostModified())
        prefs.mRepost = mNotificationPrefs->getPrefs()->mRepost;
    if (mNotificationPrefs->isRepostViaRepostModified())
        prefs.mRepostViaRepost = mNotificationPrefs->getPrefs()->mRepostViaRepost;
    if (mNotificationPrefs->isSubscribedPostModified())
        prefs.mSubscribedPost = mNotificationPrefs->getPrefs()->mSubscribedPost;

    bskyClient()->putNotificationPreferencesV2(prefs,
        [presence=getPresence()](auto){
            if (!presence)
                return;

            qDebug() << "Notifications preferences saved.";
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "putNotificationPreferencesV2 failed:" << error << " - " << msg;
            mSkywalker->showStatusMessage(tr("Failed to save notification preferences"), QEnums::STATUS_LEVEL_ERROR);
        });
}

}
