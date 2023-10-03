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

}
