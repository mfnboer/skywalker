// License: GPLv3
#pragma once
#include <QObject>
#include <QString>

namespace Skywalker {

class NotificationBackend : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void notify(const QString& title, const QString& body) = 0;

signals:
    void activated();
};

NotificationBackend* createNotificationBackend(QObject* parent);

}
