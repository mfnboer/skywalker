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

void LocalPostModelChanges::updatePostIndexedSecondsAgo()
{
    // No real changes, just signal change to refresh
    postIndexedSecondsAgoChanged();
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

void LocalPostModelChanges::updateQuoteCountDelta(const QString& cid, int delta)
{
    mChanges[cid].mQuoteCountDelta += delta;
    quoteCountChanged();
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

void LocalPostModelChanges::updateLikeTransient(const QString& cid, bool transient)
{
    mChanges[cid].mLikeTransient = transient;
    likeTransientChanged();
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

void LocalPostModelChanges::updateHiddenReplies(const QString& cid, const QStringList& hiddenReplies)
{
    mChanges[cid].mHiddenReplies = hiddenReplies;
    hiddenRepliesChanged();
}

void LocalPostModelChanges::updatePostVideoTranscodedSource(const QString& cid, const QString& source)
{
    mChanges[cid].mPostVideoTranscodedSource = source;
    postVideoPostVideoTranscodedSourceChanged();
}

void LocalPostModelChanges::updateThreadMuted(const QString& uri, bool muted)
{
    mUriChanges[uri].mThreadMuted = muted;
    threadMutedChanged();
}

bool LocalPostModelChanges::updateDetachedRecord(const QString& cid, const QString& postUri)
{
    if (postUri.isEmpty())
    {
        if (!mChanges[cid].mDetachedRecord)
        {
            // Was not locally detached, get record from network
            return true;
        }
        else
        {
            mChanges[cid].mDetachedRecord = nullptr; // re-attach of a previously detached record
        }
    }
    else
    {
        mChanges[cid].mDetachedRecord = RecordView::makeDetachedRecord(postUri);
    }

    detachedRecordChanged();
    return false;
}

void LocalPostModelChanges::updateReAttachedRecord(const QString& cid, RecordView::SharedPtr record)
{
    mChanges[cid].mReAttachedRecord = record;
    reAttachedRecordChanged();
}

void LocalPostModelChanges::updateViewerStatePinned(const QString& cid, bool pinned)
{
    mChanges[cid].mViewerStatePinned = pinned;
    viewerStatePinnedChanged();
}

void LocalPostModelChanges::updatePostDeleted(const QString& cid)
{
    mChanges[cid].mPostDeleted = true;
    postDeletedChanged();
}

}
