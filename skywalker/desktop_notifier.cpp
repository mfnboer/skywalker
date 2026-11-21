// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "desktop_notifier.h"
#include "notification.h"
#include <QDebug>
#include <algorithm>

#ifdef Q_OS_LINUX
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QVariantMap>
#endif

namespace Skywalker {

DesktopNotifier::DesktopNotifier(QObject* parent) :
    QObject(parent)
{
}

static constexpr int MAX_SHOWN = 5;

static QString getBody(const Notification& notification)
{
    const QString text = notification.getPostRecord().isNull() ? QString() : notification.getPostRecord().getText();
    const auto withText = [&text](const QString& prefix){
        return text.isEmpty() ? prefix : prefix + "\n" + text;
    };

    switch (notification.getReason())
    {
    case Notification::Reason::NOTIFICATION_REASON_LIKE:
        return QObject::tr("Liked your post");
    case Notification::Reason::NOTIFICATION_REASON_LIKE_VIA_REPOST:
        return QObject::tr("Liked your repost");
    case Notification::Reason::NOTIFICATION_REASON_REPOST:
        return QObject::tr("Reposted your post");
    case Notification::Reason::NOTIFICATION_REASON_REPOST_VIA_REPOST:
        return QObject::tr("Reposted your repost");
    case Notification::Reason::NOTIFICATION_REASON_FOLLOW:
        return QObject::tr("Follows you");
    case Notification::Reason::NOTIFICATION_REASON_MENTION:
        return withText(QObject::tr("Mentioned you"));
    case Notification::Reason::NOTIFICATION_REASON_REPLY:
        return withText(QObject::tr("Replied to your post"));
    case Notification::Reason::NOTIFICATION_REASON_QUOTE:
        return withText(QObject::tr("Quoted your post"));
    case Notification::Reason::NOTIFICATION_REASON_SUBSCRIBED_POST:
        return withText(QObject::tr("Subscribed post"));
    case Notification::Reason::NOTIFICATION_REASON_VERIFIED:
        return QObject::tr("Verified you");
    case Notification::Reason::NOTIFICATION_REASON_UNVERIFIED:
        return QObject::tr("Deleted your verification");
    default:
        return {};
    }
}

static bool isPushEnabled(const ATProto::AppBskyNotification::Preferences& prefs,
                          ATProto::AppBskyNotification::NotificationReason reason)
{
    using Reason = ATProto::AppBskyNotification::NotificationReason;
    const auto enabled = [](const auto& pref){ return pref && pref->mPush; };

    switch (reason)
    {
    case Reason::LIKE: return enabled(prefs.mLike);
    case Reason::LIKE_VIA_REPOST: return enabled(prefs.mLikeViaRepost);
    case Reason::REPOST: return enabled(prefs.mRepost);
    case Reason::REPOST_VIA_REPOST: return enabled(prefs.mRepostViaRepost);
    case Reason::FOLLOW: return enabled(prefs.mFollow);
    case Reason::MENTION: return enabled(prefs.mMention);
    case Reason::REPLY: return enabled(prefs.mReply);
    case Reason::QUOTE: return enabled(prefs.mQuote);
    case Reason::SUBSCRIBED_POST: return enabled(prefs.mSubscribedPost);
    case Reason::VERIFIED: return enabled(prefs.mVerified);
    case Reason::UNVERIFIED: return enabled(prefs.mUnverified);
    default: return false;
    }
}

// Only notification types for which push notifications are enabled in the
// account's notification preferences are shown.
void DesktopNotifier::notifyNew(ATProto::Client* bsky, int count)
{
    if (!bsky)
        return;

    bsky->getNotificationPreferences(
        [this, bsky, count](auto prefsOutput){
            if (!prefsOutput || !prefsOutput->mPreferences)
                return;

            const auto prefs = prefsOutput->mPreferences;
            showNew(bsky, prefs, count);
        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get notification preferences:" << error << "-" << msg;
        });
}

void DesktopNotifier::showNew(ATProto::Client* bsky, ATProto::AppBskyNotification::Preferences::SharedPtr prefs, int count)
{
    const int limit = std::clamp(count, 1, 100);

    bsky->listNotifications(limit, {}, {}, {},
        [this, prefs](auto output){
            int total = 0;
            int shown = 0;

            for (const auto& raw : output->mNotifications)
            {
                if (!isPushEnabled(*prefs, raw->mReason))
                    continue;

                const Notification notification(raw);
                const QString body = getBody(notification);

                if (body.isEmpty())
                    continue;

                ++total;

                if (shown < MAX_SHOWN)
                {
                    notify(notification.getAuthor().getName(), body);
                    ++shown;
                }
            }

            if (total > shown)
                notify(tr("Skywalker"), tr("and %n more notification(s)", "", total - shown));
        },
        [](const QString& error, const QString& msg){
            qWarning() << "Failed to get notifications:" << error << "-" << msg;
        });
}

void DesktopNotifier::notify(const QString& title, const QString& body)
{
#ifdef Q_OS_LINUX
    auto bus = QDBusConnection::sessionBus();

    if (!bus.isConnected())
    {
        qWarning() << "Cannot send desktop notification: session D-Bus is unavailable";
        return;
    }

    auto message = QDBusMessage::createMethodCall(
        "org.freedesktop.Notifications",
        "/org/freedesktop/Notifications",
        "org.freedesktop.Notifications",
        "Notify");

    QVariantMap hints;
    hints.insert("desktop-entry", "skywalker");

    // No actions: clicking a notification must not affect the app window.
    message << QCoreApplication::applicationName()
            << uint(0) // replaces_id
            << QString() // icon
            << title
            << body
            << QStringList()
            << hints
            << qint32(-1); // server default timeout

    auto* watcher = new QDBusPendingCallWatcher(bus.asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [watcher]{
        const QDBusPendingReply<uint> reply = *watcher;

        if (reply.isError())
            qWarning() << "Failed to send desktop notification:" << reply.error().message();

        watcher->deleteLater();
    });
#else
    Q_UNUSED(title);
    Q_UNUSED(body);
#endif
}

}
