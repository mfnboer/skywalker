// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "abstract_post_feed_model.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

using namespace std::chrono_literals;

QCache<QString, CachedBasicProfile> AbstractPostFeedModel::sAuthorCache(1000);

void AbstractPostFeedModel::cacheAuthorProfile(const QString& did, const BasicProfile& profile)
{
    sAuthorCache.insert(did, new CachedBasicProfile(profile));
}

AbstractPostFeedModel::AbstractPostFeedModel(const QString& userDid, const IProfileStore& following, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid),
    mFollowing(following)
{}

void AbstractPostFeedModel::clearFeed()
{
    mFeed.clear();
    mStoredCids.clear();
    mStoredCidQueue = {};
    mEndOfFeed = false;
    mLocalChanges.clear();
}

void AbstractPostFeedModel::storeCid(const QString& cid)
{
    mStoredCids.insert(cid);
    mStoredCidQueue.push(cid);
}

void AbstractPostFeedModel::removeStoredCid(const QString& cid)
{
    // We leave the cid's in the chronological queues. They don't do
    // harm. At cleanup, the non-stored cid's will be removed from the queue.
    mStoredCids.erase(cid);
}

void AbstractPostFeedModel::cleanupStoredCids()
{
    while (mStoredCids.size() > MAX_TIMELINE_SIZE && !mStoredCidQueue.empty())
    {
        const auto& cid = mStoredCidQueue.front();
        mStoredCids.erase(cid);
        mStoredCidQueue.pop();
    }

    if (mStoredCidQueue.empty())
    {
        Q_ASSERT(mStoredCids.empty());
        qWarning() << "Stored cid set should have been empty:" << mStoredCids.size();
        mStoredCids.clear();
    }

    qDebug() << "Stored cid set:" << mStoredCids.size() << "cid queue:" << mStoredCidQueue.size();
}

int AbstractPostFeedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mFeed.size();
}

QVariant AbstractPostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mFeed.size())
        return {};

    const auto& post = mFeed[index.row()];
    const auto* change = mLocalChanges.getChange(post.getCid());

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(post.getAuthor());
    case Role::PostText:
        return post.getFormattedText();
    case Role::PostUri:
        return post.getUri();
    case Role::PostCid:
        return post.getCid();
    case Role::PostIndexedDateTime:
        return post.getIndexedAt();
    case Role::PostImages:
    {
        QList<ImageView> images;
        for (const auto& img : post.getImages())
            images.push_back(*img);

        return QVariant::fromValue(images);
    }
    case Role::PostExternal:
    {
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant();
    }
    case Role::PostRepostedByName:
    {
        const auto& repostedBy = post.getRepostedBy();
        return repostedBy ? repostedBy->getName() : QVariant();
    }
    case Role::PostRecord:
    {
        auto record = post.getRecordView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::PostRecordWithMedia:
    {
        auto record = post.getRecordWithMediaView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::PostType:
        return post.getPostType();
    case Role::PostThreadType:
        return post.getThreadType();
    case Role::PostIsPlaceHolder:
        return post.isPlaceHolder();
    case Role::PostGapId:
        return post.getGapId();
    case Role::PostNotFound:
        return post.isNotFound();
    case Role::PostBlocked:
        return post.isBlocked();
    case Role::PostNotSupported:
        return post.isNotSupported();
    case Role::PostUnsupportedType:
        return post.getUnsupportedType();
    case Role::PostIsReply:
        return post.isReply();
    case Role::PostParentInThread:
        return post.isParentInThread();
    case Role::PostReplyToAuthor:
    {
        const auto& author = post.getReplyToAuthor();
        return author ? QVariant::fromValue(*author) : QVariant();
    }
    case Role::PostReplyRootCid:
        return post.getReplyRootCid();
    case Role::PostReplyRootUri:
        return post.getReplyRootUri();
    case Role::PostReplyCount:
        return post.getReplyCount() + (change ? change->mReplyCountDelta : 0);
    case Role::PostRepostCount:
        return post.getRepostCount() + (change ? change->mRepostCountDelta : 0);
    case Role::PostLikeCount:
        return post.getLikeCount() + (change ? change->mLikeCountDelta : 0);
    case Role::PostRepostUri:
        return change && change->mRepostUri ? *change->mRepostUri : post.getRepostUri();
    case Role::PostLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : post.getLikeUri();
    case Role::PostLocallyDeleted:
    {
        if (!change || !change->mRepostUri)
            return false;

        auto repostedBy = post.getRepostedBy();
        if (!repostedBy)
            return false;

        if (repostedBy->getDid() != mUserDid)
            return false;

        return post.getRepostUri() != *change->mRepostUri;
    }
    case Role::EndOfFeed:
        return post.isEndOfFeed();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

QHash<int, QByteArray> AbstractPostFeedModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Author), "author" },
        { int(Role::PostUri), "postUri" },
        { int(Role::PostCid), "postCid" },
        { int(Role::PostText), "postText" },
        { int(Role::PostIndexedDateTime), "postIndexedDateTime" },
        { int(Role::PostRepostedByName), "postRepostedByName" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostExternal), "postExternal" },
        { int(Role::PostRecord), "postRecord" },
        { int(Role::PostRecordWithMedia), "postRecordWithMedia" },
        { int(Role::PostType), "postType" },
        { int(Role::PostThreadType), "postThreadType" },
        { int(Role::PostIsPlaceHolder), "postIsPlaceHolder" },
        { int(Role::PostGapId), "postGapId" },
        { int(Role::PostNotFound), "postNotFound" },
        { int(Role::PostBlocked), "postBlocked" },
        { int(Role::PostNotSupported), "postNotSupported" },
        { int(Role::PostUnsupportedType), "postUnsupportedType" },
        { int(Role::PostIsReply), "postIsReply" },
        { int(Role::PostParentInThread), "postParentInThread" },
        { int(Role::PostReplyToAuthor), "postReplyToAuthor" },
        { int(Role::PostReplyRootUri), "postReplyRootUri" },
        { int(Role::PostReplyRootCid), "postReplyRootCid" },
        { int(Role::PostReplyCount), "postReplyCount" },
        { int(Role::PostRepostCount), "postRepostCount" },
        { int(Role::PostLikeCount), "postLikeCount" },
        { int(Role::PostRepostUri), "postRepostUri" },
        { int(Role::PostLikeUri), "postLikeUri" },
        { int(Role::PostLocallyDeleted), "postLocallyDeleted" },
        { int(Role::EndOfFeed), "endOfFeed" }
    };

    return roles;
}

void AbstractPostFeedModel::updatePostIndexTimestamps()
{
    changeData({ int(Role::PostIndexedDateTime) });
}

// For a change on a single post a single row change would be sufficient.
// However the model may have changed while waiting for a confirmation from the
// network about the change, so we don't know which row this CID is anymore.
// Also for reposts a CID may apply to multiple rows. Therefor all rows
// are marked as change. Performance seems not an issue.
void AbstractPostFeedModel::updateReplyCountDelta(const QString& cid, int delta)
{
    auto& change = mLocalChanges.getChangeForUpdate(cid);
    change.mReplyCountDelta += delta;
    changeData({ int(Role::PostReplyCount) });
}

void AbstractPostFeedModel::updateRepostCountDelta(const QString& cid, int delta)
{
    auto& change = mLocalChanges.getChangeForUpdate(cid);
    change.mRepostCountDelta += delta;
    changeData({ int(Role::PostRepostCount) });
}

void AbstractPostFeedModel::updateRepostUri(const QString& cid, const QString& repostUri)
{
    auto& change = mLocalChanges.getChangeForUpdate(cid);
    change.mRepostUri = repostUri;
    changeData({ int(Role::PostRepostUri), int(Role::PostLocallyDeleted) });
}

void AbstractPostFeedModel::updateLikeCountDelta(const QString& cid, int delta)
{
    auto& change = mLocalChanges.getChangeForUpdate(cid);
    change.mLikeCountDelta += delta;
    changeData({ int(Role::PostLikeCount) });
}

void AbstractPostFeedModel::updateLikeUri(const QString& cid, const QString& likeUri)
{
    auto& change = mLocalChanges.getChangeForUpdate(cid);
    change.mLikeUri = likeUri;
    changeData({ int(Role::PostLikeUri) });
}

void AbstractPostFeedModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mFeed.size() - 1, 0), roles);
}

}
