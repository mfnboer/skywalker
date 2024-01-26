// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "local_post_model_changes.h"

namespace Skywalker {

const LocalPostModelChanges::Change* LocalPostModelChanges::getLocalChange(const QString& cid) const
{
    auto it = mChanges.find(cid);
    return it != mChanges.end() ? &it->second : nullptr;
}

const BasicProfile* LocalPostModelChanges::getProfileChange(const QString& did) const
{
    auto it = mProfileChanges.find(did);
    return it != mProfileChanges.end() ? &it->second : nullptr;
}

void LocalPostModelChanges::clearLocalChanges()
{
    mChanges.clear();
    mProfileChanges.clear();
}

void LocalPostModelChanges::updatePostIndexTimestamps()
{
    // No real changes, just signal change to refresh
    postIndexTimestampChanged();
}

void LocalPostModelChanges::updateReplyCountDelta(const QString& cid, int delta)
{
    mChanges[cid].mReplyCountDelta += delta;
    replyCountChanged();
}

void LocalPostModelChanges::updateRepostCountDelta(const QString& cid, int delta)
{
    mChanges[cid].mRepostCountDelta += delta;
    repostCountChanged();
}

void LocalPostModelChanges::updateRepostUri(const QString& cid, const QString& repostUri)
{
    mChanges[cid].mRepostUri = repostUri;
    repostUriChanged();
}

void LocalPostModelChanges::updateLikeCountDelta(const QString& cid, int delta)
{
    mChanges[cid].mLikeCountDelta += delta;
    likeCountChanged();
}

void LocalPostModelChanges::updateLikeUri(const QString& cid, const QString& likeUri)
{
    mChanges[cid].mLikeUri = likeUri;
    likeUriChanged();
}

void LocalPostModelChanges::updatePostDeleted(const QString& cid)
{
    mChanges[cid].mPostDeleted = true;
    postDeletedChanged();
}

void LocalPostModelChanges::updateProfile(const BasicProfile& profile)
{
    mProfileChanges[profile.getDid()] = profile.nonVolatileCopy();
    profileChanged();
}

}
