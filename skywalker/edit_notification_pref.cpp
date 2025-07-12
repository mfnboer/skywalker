// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "edit_notification_pref.h"

namespace Skywalker {

EditNotificationPref::EditNotificationPref(QObject* parent) :
    QObject(parent)
{
}

EditNotificationPref::EditNotificationPref(const ATProto::AppBskyNotification::Preference::SharedPtr& pref, QObject* parent) :
    QObject(parent),
    mPref(pref)
{
}

void EditNotificationPref::setList(bool list)
{
    if (list != getList())
    {
        mPref->mList = list;
        mModified = true;
        emit listChanged();
    }
}

void EditNotificationPref::setPush(bool push)
{
    if (push != getPush())
    {
        mPref->mPush = push;
        mModified = true;
        emit pushChanged();
    }
}

}
