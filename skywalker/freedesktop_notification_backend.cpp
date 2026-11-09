// License: GPLv3
#include "notification_backend.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDebug>
#include <QSet>
#include <QStringList>
#include <QTextDocumentFragment>
#include <QVariantMap>

namespace Skywalker {
namespace {

constexpr auto NOTIFICATIONS_SERVICE = "org.freedesktop.Notifications";
constexpr auto NOTIFICATIONS_PATH = "/org/freedesktop/Notifications";
constexpr auto NOTIFICATIONS_INTERFACE = "org.freedesktop.Notifications";

}

class FreedesktopNotificationBackend final : public NotificationBackend
{
    Q_OBJECT
public:
    FreedesktopNotificationBackend(QObject* parent) : NotificationBackend(parent)
    {
        const auto sessionBus = QDBusConnection::sessionBus();

        if (!sessionBus.isConnected())
        {
            qWarning() << "Cannot connect to the session D-Bus for desktop notifications";
        }
        else if (!sessionBus.connect(
                     NOTIFICATIONS_SERVICE,
                     NOTIFICATIONS_PATH,
                     NOTIFICATIONS_INTERFACE,
                     "ActionInvoked",
                     this,
                     SLOT(actionInvoked(uint,QString))))
        {
            qWarning() << "Cannot subscribe to desktop notification actions";
        }

        if (sessionBus.isConnected() &&
            !sessionBus.connect(
                NOTIFICATIONS_SERVICE,
                NOTIFICATIONS_PATH,
                NOTIFICATIONS_INTERFACE,
                "NotificationClosed",
                this,
                SLOT(notificationClosed(uint,uint))))
        {
            qWarning() << "Cannot subscribe to desktop notification close events";
        }
    }

    void notify(const QString& title, const QString& body) override
    {
        const auto sessionBus = QDBusConnection::sessionBus();

        if (!sessionBus.isConnected())
        {
            qWarning() << "Cannot send desktop notification: session D-Bus is unavailable";
            return;
        }

        auto message = QDBusMessage::createMethodCall(
            NOTIFICATIONS_SERVICE,
            NOTIFICATIONS_PATH,
            NOTIFICATIONS_INTERFACE,
            "Notify");

        const QStringList actions{ "default", tr("Open") };
        QVariantMap hints;
        hints.insert("desktop-entry", "skywalker");

        message << QCoreApplication::applicationName()
                << uint(0)
                << QString()
                << title
                << QTextDocumentFragment::fromHtml(body).toPlainText()
                << actions
                << hints
                << qint32(-1);

        auto* watcher = new QDBusPendingCallWatcher(sessionBus.asyncCall(message), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]{
            const QDBusPendingReply<uint> reply = *watcher;

            if (reply.isError())
            {
                qWarning() << "Failed to send desktop notification:" << reply.error().message();
            }
            else
            {
                mNotificationIds.insert(reply.value());
            }

            watcher->deleteLater();
        });
    }

private slots:
    void actionInvoked(uint notificationId, const QString& actionKey)
    {
        if (mNotificationIds.remove(notificationId) && actionKey == "default")
            emit activated();
    }

    void notificationClosed(uint notificationId, uint)
    {
        mNotificationIds.remove(notificationId);
    }

private:
    QSet<uint> mNotificationIds;
};

NotificationBackend* createNotificationBackend(QObject* parent)
{
    return new FreedesktopNotificationBackend(parent);
}

}

#include "freedesktop_notification_backend.moc"
