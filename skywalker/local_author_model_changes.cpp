// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "local_author_model_changes.h"

namespace Skywalker {

const LocalAuthorModelChanges::Change* LocalAuthorModelChanges::getLocalChange(const QString& did) const
{
    auto it = mChanges.find(did);
    return it != mChanges.end() ? &it->second : nullptr;
}

void LocalAuthorModelChanges::clearLocalChanges()
{
    mChanges.clear();
}

void LocalAuthorModelChanges::updateBlockingUri(const QString& did, const QString& blockingUri)
{
    mChanges[did].mBlockingUri = blockingUri;
    blockingUriChanged();
}

void LocalAuthorModelChanges::updateFollowingUri(const QString& did, const QString& followingUri)
{
    mChanges[did].mFollowingUri = followingUri;
    followingUriChanged();
}

void LocalAuthorModelChanges::updateActivitySubscription(const QString& did, const ActivitySubscription& subscription)
{
    mChanges[did].mActivitySubscription = subscription;
    activitySubscriptionChanged();
}

void LocalAuthorModelChanges::updateMuted(const QString& did, bool muted)
{
    mChanges[did].mMuted = muted;
    mutedChanged();
}

void LocalAuthorModelChanges::updateMutedReposts(const QString& did, bool mutedReposts)
{
    mChanges[did].mMutedReposts = mutedReposts;
    mutedRepostsChanged();
}

void LocalAuthorModelChanges::updateHideFromTimeline()
{
    // No need to actual store the changed value here as the value in mTimelineHide
    // is updated. The changed() signal will trigger retrieving the new value.
    // Probably updateMutedReposts can be changed likewise.
    hideFromTimelineChanged();
}

}
