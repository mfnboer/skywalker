// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_inbox.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto NOTIFICATION_REFRESH_INTERVAL = 907s;

OzoneInbox::OzoneInbox(QObject* parent) : WrappedSkywalker(parent)
{
    connect(&mRefreshNotificationTimer, &QTimer::timeout, this, [this]{ getUnreadCounts(); });
}

void OzoneInbox::clear()
{
    setUserAccountStatus({});
    setUnreadReports(0);
    setUnreadSubjects(0);
}

void OzoneInbox::updateUserAccountStatus(bool updateSeen)
{
    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetAccountStatus(
        [this, presence=getPresence(), updateSeen](ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr output){
            if (!presence)
                return;

            const OzoneAccountStatus status(output);
            setUserAccountStatus(status);

            if (updateSeen)
                updateNotificationSeen(ATProto::ToolsOzoneInbox::OzoneInboxSection::REPORTS, status.getUpdatedAt());
        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get account status:" << error << " - " << msg;
        });
}

void OzoneInbox::getUnreadCounts()
{
    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxGetUnreadCounts(
        [this, presence=getPresence()](ATProto::ToolsOzoneInbox::UnreadCounts::SharedPtr counts){
            if (!presence)
                return;

            setUnreadReports(counts->mReports.value_or(0));
            setUnreadSubjects(counts->mSubjects.value_or(0));

            if (counts->mAccountStatus.value_or(0) > 0)
            {
                qDebug() << "Update account status:" << counts->mAccountStatus.value_or(0);
                updateUserAccountStatus(true);
            }

        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get unread counts:" << error << " - " << msg;
        });
}

void OzoneInbox::updateNotificationSeen(const QString& section, QDateTime seenAt)
{
    if (!bskyClient())
        return;

    bskyClient()->ozoneInboxUpdateSeen({section}, seenAt, {}, {});
}

void OzoneInbox::startRefreshTimers()
{
    qDebug() << "Start refresh timers";
    getUnreadCounts();
    mRefreshNotificationTimer.start(NOTIFICATION_REFRESH_INTERVAL);
}

void OzoneInbox::stopRefreshTimers()
{
    qDebug() << "Stop refresh timers";
    mRefreshNotificationTimer.stop();
}

void OzoneInbox::setUserAccountStatus(const OzoneAccountStatus& status)
{
    mUserAccountStatus = status;
    emit userAccountStatusChanged();
}

void OzoneInbox::setUnreadReports(int unread)
{
    if (unread != mUnreadReports)
    {
        mUnreadReports = unread;
        emit unreadReportsChanged();
    }
}

void OzoneInbox::setUnreadSubjects(int unread)
{
    if (unread != mUnreadSubjects)
    {
        mUnreadSubjects = unread;
        emit unreadSubjectsChanged();
    }
}

}
