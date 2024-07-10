// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "local_profile_changes.h"

namespace Skywalker {

const Profile* LocalProfileChanges::getProfileChange(const QString& did) const
{
    auto it = mChanges.find(did);
    return it != mChanges.end() ? &it->second : nullptr;
}

void LocalProfileChanges::clearLocalProfileChanges()
{
    mChanges.clear();
}

void LocalProfileChanges::updateProfile(const Profile& profile)
{
    mChanges[profile.getDid()] = profile;
    profileChanged();
}

}
