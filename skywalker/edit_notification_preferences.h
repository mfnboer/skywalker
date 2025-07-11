// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "edit_notification_filterable_pref.h"
#include "enums.h"
#include <atproto/lib/lexicon/app_bsky_notification.h>
#include <QObject>

namespace Skywalker {

class EditNotificationPreferences : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QEnums::NotifcationChatIncludeType chatIncludeType READ getChatIncludeType WRITE setChatIncludeType NOTIFY chatIncludeTypeChanged FINAL)
    Q_PROPERTY(bool chatPush READ getChatPush WRITE setChatPush NOTIFY chatPushChanged FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* follow READ getFollow CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* like READ getLike CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* likeViaRepost READ getLikeViaRepost CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* mention READ getMention CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* quote READ getQuote CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* reply READ getReply CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* repost READ getRepost CONSTANT FINAL)
    Q_PROPERTY(EditNotificationFilterablePref* repostViaRepost READ getRepostViaRepost CONSTANT FINAL)
    QML_ELEMENT

public:
    EditNotificationPreferences(QObject* parent = nullptr);
    explicit EditNotificationPreferences(const ATProto::AppBskyNotification::Preferences::SharedPtr& prefs, QObject* parent = nullptr);

    QEnums::NotifcationChatIncludeType getChatIncludeType() const { return (QEnums::NotifcationChatIncludeType)mPrefs->mChat->mInclude; }
    void setChatIncludeType(QEnums::NotifcationChatIncludeType includeType);

    bool getChatPush() const { return mPrefs->mChat->mPush; }
    void setChatPush(bool push);

    bool isChatModified() const { return mChatModified; }

    EditNotificationFilterablePref* getFollow() const { return mFollowPref.get(); }
    bool isFollowModified() const { return mFollowPref->isModified(); }

    EditNotificationFilterablePref* getLike() const { return mLikePref.get(); }
    bool isLikeModified() const { return mLikePref->isModified(); }

    EditNotificationFilterablePref* getLikeViaRepost() const { return mLikeViaRepostPref.get(); }
    bool isLikeViaRepostModified() const { return mLikeViaRepostPref->isModified(); }

    EditNotificationFilterablePref* getMention() const { return mMentionPref.get(); }
    bool isMentionModified() const { return mMentionPref->isModified(); }

    EditNotificationFilterablePref* getQuote() const { return mQuotePref.get(); }
    bool isQuoteModified() const { return mQuotePref->isModified(); }

    EditNotificationFilterablePref* getReply() const { return mReplyPref.get(); }
    bool isReplyModified() const { return mReplyPref->isModified(); }

    EditNotificationFilterablePref* getRepost() const { return mRepostPref.get(); }
    bool isRepostModified() const { return mRepostPref->isModified(); }

    EditNotificationFilterablePref* getRepostViaRepost() const { return mRepostViaRepostPref.get(); }
    bool isRepostViaRepostModified() const { return mRepostViaRepostPref->isModified(); }

signals:
    void chatIncludeTypeChanged();
    void chatPushChanged();

private:
    ATProto::AppBskyNotification::Preferences::SharedPtr mPrefs;
    bool mChatModified = false;

    std::unique_ptr<EditNotificationFilterablePref> mFollowPref;
    std::unique_ptr<EditNotificationFilterablePref> mLikePref;
    std::unique_ptr<EditNotificationFilterablePref> mLikeViaRepostPref;
    std::unique_ptr<EditNotificationFilterablePref> mMentionPref;
    std::unique_ptr<EditNotificationFilterablePref> mQuotePref;
    std::unique_ptr<EditNotificationFilterablePref> mReplyPref;
    std::unique_ptr<EditNotificationFilterablePref> mRepostPref;
    std::unique_ptr<EditNotificationFilterablePref> mRepostViaRepostPref;
};

}
