// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "local_feed_model_changes.h"

namespace Skywalker {

const LocalFeedModelChanges::Change* LocalFeedModelChanges::getLocalChange(const QString& cid) const
{
    auto it = mChanges.find(cid);
    return it != mChanges.end() ? &it->second : nullptr;
}

void LocalFeedModelChanges::clearLocalChanges()
{
    mChanges.clear();
}

void LocalFeedModelChanges::updateLikeCountDelta(const QString& cid, int delta)
{
    mChanges[cid].mLikeCountDelta += delta;
    likeCountChanged();
}

void LocalFeedModelChanges::updateLikeUri(const QString& cid, const QString& likeUri)
{
    mChanges[cid].mLikeUri = likeUri;
    likeUriChanged();
}

}
