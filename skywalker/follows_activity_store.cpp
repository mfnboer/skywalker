// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "follows_activity_store.h"

namespace Skywalker {

using namespace std::chrono_literals;

static constexpr auto UPDATE_INTERVAL = 31s;

FollowsActivityStore::FollowsActivityStore(const IProfileStore& userFollows, QObject* parent) :
    QObject(parent),
    mUserFollows(userFollows)
{
    connect(&mUpdateTimer, &QTimer::timeout, this, [this]{ updateActivities(); });
    mUpdateTimer.start(UPDATE_INTERVAL);
}

void FollowsActivityStore::clear()
{
    mDidStatus.clear();
}

ActivityStatus* FollowsActivityStore::getActivityStatus(const QString& did)
{
    if (!mUserFollows.contains(did))
        return &mNotActiveStatus;

    auto it = mDidStatus.find(did);

    if (it != mDidStatus.end())
        return it->second;

    // TODO: delete on unfollow
    auto* status = new ActivityStatus(did, this);
    mDidStatus.insert({did, status});
    return status;
}

void FollowsActivityStore::reportActivity(const QString& did, QDateTime timestamp)
{
    if (!mUserFollows.contains(did))
        return;

    auto* status = getActivityStatus(did);
    mActiveStatusSet.erase(status);
    status->setLastActive(timestamp);

    if (status->isActive())
        mActiveStatusSet.insert(status);
}

void FollowsActivityStore::updateActivities()
{
    const auto now = QDateTime::currentDateTimeUtc();

    for (auto it = mActiveStatusSet.begin(); it != mActiveStatusSet.end(); )
    {
        auto* status = *it;
        status->updateActive(now);

        if (!status->isActive())
            it = mActiveStatusSet.erase(it);
        else
            break; // the remainder is newer and hence will stay active
    }
}

}
