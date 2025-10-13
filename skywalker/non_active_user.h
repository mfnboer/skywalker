// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"

namespace Skywalker {

class NonActiveUser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BasicProfile profile READ getProfile CONSTANT FINAL)
    Q_PROPERTY(int unreadNotificationCount READ getUnreadNotificationCount WRITE setUnreadNotificationCount NOTIFY unreadNotificationCountChanged FINAL)
    Q_PROPERTY(bool sessionExpired READ isSessionExpired CONSTANT FINAL)

public:
    using List = QList<NonActiveUser*>;

    NonActiveUser(const BasicProfile& profile, bool sessionExpired, QObject* parent = nullptr);

    const BasicProfile& getProfile() const { return mProfile; }
    int getUnreadNotificationCount() const { return mUnreadNotificationCount; }
    void setUnreadNotificationCount(int unread);
    bool isSessionExpired() const { return mSessionExpired; }

signals:
    void unreadNotificationCountChanged();

private:
    BasicProfile mProfile;
    int mUnreadNotificationCount = 0;
    bool mSessionExpired = false;
};

}
