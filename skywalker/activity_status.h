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
    Q_PROPERTY(QDateTime lastActive READ getLastActive NOTIFY lastActiveChanged FINAL)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    QML_ELEMENT
    QML_UNCREATABLE("only created on the C++ side")

public:
    static const std::chrono::minutes ACTIVE_INTERVAL;

    explicit ActivityStatus(const QString& did, QObject* parent = nullptr);

    const QString& getDid() const { return mDid; }

    QDateTime getLastActive() const { return mLastActive; }
    void setLastActive(QDateTime lastActive);

    bool isActive() const;
    void updateActive(QDateTime now);

    bool operator<(const ActivityStatus& rhs) const;

signals:
    void lastActiveChanged();
    void activeChanged();

private:
    QString mDid; // can be empty for default status
    QDateTime mLastActive;
    QDateTime mActivityCheck;
};

struct ActiviyStatusPtrCmp
{
    bool operator()(ActivityStatus* lhs, ActivityStatus* rhs) const
    {
        return *lhs < *rhs;
    }
};

}
