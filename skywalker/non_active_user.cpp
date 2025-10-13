// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "non_active_user.h"

namespace Skywalker {

NonActiveUser::NonActiveUser(const BasicProfile& profile, bool sessionExpired, QObject* parent) :
    QObject(parent),
    mProfile(profile),
    mSessionExpired(sessionExpired)
{
}

void NonActiveUser::setUnreadNotificationCount(int unread)
{
    if (unread == mUnreadNotificationCount)
        return;

    mUnreadNotificationCount = unread;
    emit unreadNotificationCountChanged();
}

}
