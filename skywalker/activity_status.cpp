// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "activity_status.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto ACTIVE_INTERVAL = 10min;

ActivityStatus::ActivityStatus(const QString& did, QObject* parent) :
    QObject(parent),
    mDid(did)
{
}

void ActivityStatus::setLastActive(QDateTime lastActive)
{
    Q_ASSERT(lastActive.isValid());

    if (!mLastActive.isValid() || lastActive > mLastActive)
    {
        const bool wasActive = isActive();

        mLastActive = lastActive;
        emit lastActiveChanged();

        mActivityCheck = QDateTime::currentDateTimeUtc();

        if (isActive() != wasActive)
            emit activeChanged();
    }
}

bool ActivityStatus::isActive() const
{
    if (!mLastActive.isValid())
        return false;

    return mActivityCheck - mLastActive < ACTIVE_INTERVAL;
}

void ActivityStatus::updateActive(QDateTime now)
{
    const bool wasActive = isActive();
    mActivityCheck = now;

    if (isActive() != wasActive)
        emit activeChanged();
}

bool ActivityStatus::operator<(const ActivityStatus& rhs) const
{
    if (!mLastActive.isValid())
        return rhs.mLastActive.isValid();

    if (!rhs.mLastActive.isValid())
        return false;

    if (mLastActive != rhs.mLastActive)
        return mLastActive < rhs.mLastActive;

    return mDid < rhs.mDid;
}

}
