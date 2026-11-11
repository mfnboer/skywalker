// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "ozone_account_status.h"
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class OzoneInbox : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(OzoneAccountStatus userAccountStatus READ getUserAccountStatus NOTIFY userAccountStatusChanged FINAL)
    Q_PROPERTY(int unreadReports READ getUnreadReports NOTIFY unreadReportsChanged FINAL)
    Q_PROPERTY(int unreadSubjects READ getUnreadSubjects NOTIFY unreadSubjectsChanged FINAL)
    QML_ELEMENT

public:
    explicit OzoneInbox(QObject* parent = nullptr);

    void clear();
    OzoneAccountStatus getUserAccountStatus() const { return mUserAccountStatus; }
    int getUnreadReports() const { return mUnreadReports; }
    int getUnreadSubjects() const { return mUnreadSubjects; }
    void updateUserAccountStatus(bool updateSeen = false);
    void getUnreadCounts();
    void updateNotificationSeen(const QString& section, QDateTime seenAt);

    void startRefreshTimers();
    void stopRefreshTimers();

    void setUnreadReports(int unread);
    void setUnreadSubjects(int unread);

signals:
    void userAccountStatusChanged();
    void unreadReportsChanged();
    void unreadSubjectsChanged();

private:
    void setUserAccountStatus(const OzoneAccountStatus& status);

    OzoneAccountStatus mUserAccountStatus;
    int mUnreadReports = 0;
    int mUnreadSubjects = 0;
    QTimer mRefreshNotificationTimer;
};

}