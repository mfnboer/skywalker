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

            mNotificationPrefs = prefs->mPreferences;
            getNotificationDeclaration();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getNotificationPreferences failed:" << error << " - " << msg;
            emit notificationPrefsFailed(msg);
        });
}

void NotificationUtils::getNotificationDeclaration()
{
    if (!notificationMaster())
        return;

    notificationMaster()->getDeclaration(mSkywalker->getUserDid(),
        [this, presence=getPresence()](ATProto::AppBskyNotification::Declaration::SharedPtr declaration){
            if (!presence)
                return;

            const auto allowSubscriptions = (QEnums::AllowActivitySubscriptionsType)declaration->mAllowSubscriptions;
            qDebug() << "Allow subscriptions:" << allowSubscriptions;

            mEditNotificationPrefs = std::make_unique<EditNotificationPreferences>(mNotificationPrefs, allowSubscriptions, this);
            emit notificationPrefsOk(mEditNotificationPrefs.get());
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to get notification declaration:" << error << "-" << msg;
            mEditNotificationPrefs = std::make_unique<EditNotificationPreferences>(mNotificationPrefs, QEnums::ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS, this);
            emit notificationPrefsOk(mEditNotificationPrefs.get());
        });
}

void NotificationUtils::saveNotificationPrefs()
{   
    qDebug() << "Save notification preferences";

    if (!mEditNotificationPrefs || mEditNotificationPrefs->isNull())
    {
        qDebug() << "No preferences loaded.";
        return;
    }

    if (mEditNotificationPrefs->isAllowSubscriptionsModified())
        updateNotificationDeclaration(mEditNotificationPrefs->getAllowSubscriptions());

    if (!arePreferencesModified())
    {
        qDebug() << "No modified preferences.";
        return;
    }

    ATProto::AppBskyNotification::Preferences prefs;

    if (mEditNotificationPrefs->isChatModified())
        prefs.mChat = mEditNotificationPrefs->getPrefs()->mChat;
    if (mEditNotificationPrefs->isFollowModified())
        prefs.mFollow = mEditNotificationPrefs->getPrefs()->mFollow;
    if (mEditNotificationPrefs->isLikeModified())
        prefs.mLike = mEditNotificationPrefs->getPrefs()->mLike;
    if (mEditNotificationPrefs->isLikeViaRepostModified())
        prefs.mLikeViaRepost = mEditNotificationPrefs->getPrefs()->mLikeViaRepost;
    if (mEditNotificationPrefs->isMentionModified())
        prefs.mMention = mEditNotificationPrefs->getPrefs()->mMention;
    if (mEditNotificationPrefs->isQuoteModified())
        prefs.mQuote = mEditNotificationPrefs->getPrefs()->mQuote;
    if (mEditNotificationPrefs->isReplyModified())
        prefs.mReply = mEditNotificationPrefs->getPrefs()->mReply;
    if (mEditNotificationPrefs->isRepostModified())
        prefs.mRepost = mEditNotificationPrefs->getPrefs()->mRepost;
    if (mEditNotificationPrefs->isRepostViaRepostModified())
        prefs.mRepostViaRepost = mEditNotificationPrefs->getPrefs()->mRepostViaRepost;
    if (mEditNotificationPrefs->isSubscribedPostModified())
        prefs.mSubscribedPost = mEditNotificationPrefs->getPrefs()->mSubscribedPost;

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
            mSkywalker->showStatusMessage(tr("Failed to save notification settings"), QEnums::STATUS_LEVEL_ERROR);
        });
}

void NotificationUtils::updateNotificationDeclaration(QEnums::AllowActivitySubscriptionsType allowSubscriptions)
{
    qDebug() << "Update notification declaration";

    if (!notificationMaster())
        return;

    ATProto::AppBskyNotification::Declaration declaration;
    declaration.mAllowSubscriptions = (ATProto::AppBskyActor::AllowSubscriptionsType)allowSubscriptions;

    notificationMaster()->updateDeclaration(mSkywalker->getUserDid(), declaration,
        []{
            qDebug() << "Updated declaration";
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to update notification declaration:" << error << "-" << msg;
            mSkywalker->showStatusMessage(tr("Failed to save notification settings"), QEnums::STATUS_LEVEL_ERROR);
        });
}

void NotificationUtils::subscribeActivity(const QString& did, bool posts, bool replies)
{
    if (!bskyClient())
        return;

    ATProto::AppBskyNotification::ActivitySubscription subscription;
    subscription.mPost = posts;
    subscription.mReply = replies;

    bskyClient()->putActivitySubscription(did, subscription,
        [this, presence=getPresence()](ATProto::AppBskyNotification::SubjectActivitySubscription::SharedPtr activitySubscription){
            if (!presence)
                return;

            const auto newSubscription = ActivitySubscription(activitySubscription->mActivitySubscription.get());

            mSkywalker->makeLocalModelChange(
                [did=activitySubscription->mSubject, newSubscription](LocalAuthorModelChanges* model){
                    model->updateActivitySubscription(did, newSubscription);
                });

            emit subscribeActivityOk(activitySubscription->mSubject, newSubscription);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qWarning() << error << "-" << msg;
            emit subscribeActivityFailed(msg);
        });
}

ATProto::NotificationMaster* NotificationUtils::notificationMaster()
{
    if (!mNotificationMaster)
    {
        if (bskyClient())
            mNotificationMaster = std::make_unique<ATProto::NotificationMaster>(*bskyClient());
        else
            qWarning() << "Bsky client not yet created";
    }

    return mNotificationMaster.get();
}

bool NotificationUtils::arePreferencesModified() const
{
    if (!mEditNotificationPrefs)
        return false;

    return mEditNotificationPrefs->isChatModified() ||
           mEditNotificationPrefs->isFollowModified() ||
           mEditNotificationPrefs->isLikeModified() ||
           mEditNotificationPrefs->isLikeViaRepostModified() ||
           mEditNotificationPrefs->isMentionModified() ||
           mEditNotificationPrefs->isQuoteModified() ||
           mEditNotificationPrefs->isReplyModified() ||
           mEditNotificationPrefs->isRepostModified() ||
           mEditNotificationPrefs->isRepostViaRepostModified() ||
           mEditNotificationPrefs->isSubscribedPostModified();
}

}
