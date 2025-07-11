// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "edit_notification_filterable_pref.h"

namespace Skywalker {

EditNotificationFilterablePref::EditNotificationFilterablePref(QObject* parent) :
    QObject(parent)
{
}

EditNotificationFilterablePref::EditNotificationFilterablePref(const ATProto::AppBskyNotification::FilterablePreference::SharedPtr& pref, QObject* parent) :
    QObject(parent),
    mPref(pref)
{
}

void EditNotificationFilterablePref::setIncludeType(QEnums::NotifcationFilterIncludeType includeType)
{
    if (includeType != getIncludeType())
    {
        mPref->mInclude = (ATProto::AppBskyNotification::FilterablePreference::IncludeType)includeType;
        mPref->mRawInclude = ATProto::AppBskyNotification::FilterablePreference::includeTypeToString(mPref->mInclude, "");
        mModified = true;
        emit includeTypeChanged();
    }
}

void EditNotificationFilterablePref::setList(bool list)
{
    if (list != getList())
    {
        mPref->mList = list;
        emit listChanged();
    }
}

void EditNotificationFilterablePref::setPush(bool push)
{
    if (push != getPush())
    {
        mPref->mPush = push;
        emit pushChanged();
    }
}

}
