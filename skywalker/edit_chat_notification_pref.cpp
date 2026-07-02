// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "edit_chat_notification_pref.h"

namespace Skywalker {

EditChatNotificationPref::EditChatNotificationPref(QObject* parent) :
    QObject(parent)
{
}

EditChatNotificationPref::EditChatNotificationPref(const ATProto::ChatBskyNotification::ChatPreference::SharedPtr& pref, QObject* parent) :
    QObject(parent),
    mPref(pref)
{
}

void EditChatNotificationPref::setIncludeType(QEnums::NotifcationFilterIncludeType includeType)
{
    if (includeType != getIncludeType())
    {
        mPref->mInclude = (ATProto::AppBskyNotification::FilterablePreference::IncludeType)includeType;
        mPref->mRawInclude = ATProto::AppBskyNotification::FilterablePreference::includeTypeToString(mPref->mInclude, "");
        mModified = true;
        emit includeTypeChanged();
    }
}

void EditChatNotificationPref::setPush(bool push)
{
    if (push != getPush())
    {
        mPref->mPush = push;
        mModified = true;
        emit pushChanged();
    }
}

}
