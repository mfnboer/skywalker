// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "desktop_notifier.h"
#include "notification.h"
#include <QDebug>

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

void DesktopNotifier::notifyNew(ATProto::Client* bsky, const QString& accountHandle, int count)
{
    const auto countOnly = [this, accountHandle, count]{
        notify(accountHandle, tr("%n new notification(s)", "", count));
    };

    if (!bsky)
    {
        countOnly();
        return;
    }

    bsky->listNotifications(std::min(count, MAX_SHOWN), {}, {}, {},
        [this, countOnly, count](auto output){
            int shown = 0;

            for (const auto& raw : output->mNotifications)
            {
                const Notification notification(raw);
                const QString body = getBody(notification);

                if (body.isEmpty())
                    continue;

                notify(notification.getAuthor().getName(), body);
                ++shown;
            }

            if (shown == 0)
                countOnly();
            else if (count > shown)
                notify(tr("Skywalker"), tr("and %n more notification(s)", "", count - shown));
        },
        [countOnly](const QString& error, const QString& msg){
            qWarning() << "Failed to get notifications:" << error << "-" << msg;
            countOnly();
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
