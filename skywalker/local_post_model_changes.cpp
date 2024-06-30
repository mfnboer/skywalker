// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "local_post_model_changes.h"

namespace Skywalker {

const LocalPostModelChanges::Change* LocalPostModelChanges::getLocalChange(const QString& cid) const
{
    auto it = mChanges.find(cid);
    return it != mChanges.end() ? &it->second : nullptr;
}

const LocalPostModelChanges::Change* LocalPostModelChanges::getLocalUriChange(const QString& uri) const
{
    auto it = mUriChanges.find(uri);
    return it != mUriChanges.end() ? &it->second : nullptr;
}

void LocalPostModelChanges::clearLocalChanges()
{
    mChanges.clear();
    mUriChanges.clear();
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

void LocalPostModelChanges::updateThreadgateUri(const QString& cid, const QString& threadgateUri)
{
    mChanges[cid].mThreadgateUri = threadgateUri;
    threadgateUriChanged();
}

void LocalPostModelChanges::updateReplyRestriction(const QString& cid, const QEnums::ReplyRestriction replyRestricion)
{
    mChanges[cid].mReplyRestriction = replyRestricion;
    replyRestrictionChanged();
}

void LocalPostModelChanges::updateReplyRestrictionLists(const QString& cid, const ListViewBasicList replyRestrictionLists)
{
    mChanges[cid].mReplyRestrictionLists = replyRestrictionLists;
    replyRestrictionListsChanged();
}

void LocalPostModelChanges::updateThreadMuted(const QString& uri, bool muted)
{
    mUriChanges[uri].mThreadMuted = muted;
    threadMutedChanged();
}

void LocalPostModelChanges::updatePostDeleted(const QString& cid)
{
    mChanges[cid].mPostDeleted = true;
    postDeletedChanged();
}

}
