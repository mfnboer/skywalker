// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "follows_activity_store.h"

namespace Skywalker {

FollowsActivityStore::FollowsActivityStore(const IProfileStore& userFollows, QObject* parent) :
    QObject(parent),
    mUserFollows(userFollows)
{
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
    auto* status = new ActivityStatus(this);
    mDidStatus.insert({did, status});
    return status;
}

void FollowsActivityStore::reportActivity(const QString& did, QDateTime timestamp)
{
    if (!mUserFollows.contains(did))
        return;

    auto* status = getActivityStatus(did);
    status->setLastActive(timestamp);
}

}
