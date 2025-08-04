// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "activity_status.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto ACTIVE_INTERVAL = 10min;

ActivityStatus::ActivityStatus(QObject* parent) :
    QObject(parent)
{
}

void ActivityStatus::setLastActive(QDateTime lastActive)
{
    Q_ASSERT(lastActive.isValid());

    if (!mLastActive.isValid() || lastActive > mLastActive)
    {
        mLastActive = lastActive;
        emit lastActiveChanged();
    }
}

bool ActivityStatus::isActive(QDateTime now) const
{
    if (!mLastActive.isValid())
        return false;

    return now - mLastActive < ACTIVE_INTERVAL;
}

}
