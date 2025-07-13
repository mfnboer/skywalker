// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <QHashFunctions>
#include <QString>
#include <optional>
#include <unordered_map>

namespace Skywalker {

class LocalAuthorModelChanges
{
public:
    struct Change
    {
        std::optional<QString> mBlockingUri;
        std::optional<QString> mFollowingUri;
        std::optional<ActivitySubscription> mActivitySubscription;
        std::optional<bool> mMuted;
        std::optional<bool> mMutedReposts;
    };

    LocalAuthorModelChanges() = default;
    virtual ~LocalAuthorModelChanges() = default;

    const Change* getLocalChange(const QString& did) const;
    void clearLocalChanges();

    void updateBlockingUri(const QString& did, const QString& blockingUri);
    void updateFollowingUri(const QString& did, const QString& followingUri);
    void updateActivitySubscription(const QString& did, const ActivitySubscription& subscription);
    void updateMuted(const QString& did, bool muted);
    void updateMutedReposts(const QString& did, bool mutedReposts);
    void updateHideFromTimeline();

protected:
    virtual void blockingUriChanged() = 0;
    virtual void followingUriChanged() = 0;
    virtual void activitySubscriptionChanged() = 0;
    virtual void mutedChanged() = 0;
    virtual void mutedRepostsChanged() = 0;
    virtual void hideFromTimelineChanged() = 0;

private:
    // Mapping from author DID to change
    std::unordered_map<QString, Change> mChanges;
};

}
