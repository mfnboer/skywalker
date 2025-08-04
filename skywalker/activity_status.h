// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QDateTime>
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class ActivityStatus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime lastActive READ getLastActive WRITE setLastActive NOTIFY lastActiveChanged FINAL)
    QML_ELEMENT

public:
    explicit ActivityStatus(QObject* parent = nullptr);

    QDateTime getLastActive() const { return mLastActive; }
    void setLastActive(QDateTime lastActive);

    Q_INVOKABLE bool isActive(QDateTime now = QDateTime::currentDateTimeUtc()) const;

signals:
    void lastActiveChanged();

private:
    QDateTime mLastActive;
};

}
