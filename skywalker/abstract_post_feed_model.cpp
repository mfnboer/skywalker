// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "abstract_post_feed_model.h"
#include "author_cache.h"
#include "content_filter.h"
#include "focus_hashtags.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

using namespace std::chrono_literals;

AbstractPostFeedModel::AbstractPostFeedModel(const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter, const Bookmarks& bookmarks,
                                             const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid),
    mFollowing(following),
    mMutedReposts(mutedReposts),
    mContentFilter(contentFilter),
    mBookmarks(bookmarks),
    mMutedWords(mutedWords),
    mFocusHashtags(focusHashtags),
    mHashtags(hashtags)
{
    connect(&mBookmarks, &Bookmarks::sizeChanged, this, [this]{ postBookmarkedChanged(); });
    connect(&AuthorCache::instance(), &AuthorCache::profileAdded, this,
            [this]{ changeData({ int(Role::PostReplyToAuthor), int(Role::PostRecord), int(Role::PostRecordWithMedia) }); });
}

void AbstractPostFeedModel::clearFeed()
{
    mFeed.clear();
    mStoredCids.clear();
    mStoredCidQueue = {};
    mEndOfFeed = false;
    clearLocalChanges();
    clearLocalProfileChanges();
}

void AbstractPostFeedModel::deletePost(int index)
{
    mFeed.erase(mFeed.begin() + index);
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

bool AbstractPostFeedModel::mustHideContent(const Post& post) const
{
    if (post.getAuthor().getViewer().isMuted())
    {
        qDebug() << "Hide post of muted author:" << post.getAuthor().getHandleOrDid() << post.getCid();
        return true;
    }

    const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels());

    if (visibility == QEnums::CONTENT_VISIBILITY_HIDE_POST)
    {
        qDebug() << "Hide post:" << post.getCid() << warning;
        return true;
    }

    if (post.isRepost() && mMutedReposts.contains(post.getRepostedBy()->getDid()))
    {
        qDebug() << "Mute repost, did:" << post.getRepostedBy()->getDid();
        return true;
    }

    if (mMutedWords.match(post))
    {
        qDebug() << "Hide post due to muted words" << post.getCid();
        return true;
    }

    return false;
}

void AbstractPostFeedModel::preprocess(const Post& post)
{
    const auto hashtags = post.getHashtags();

    for (const auto& tag : hashtags)
        mHashtags.insert(tag);
}

void AbstractPostFeedModel::unfoldPosts(int startIndex)
{
    qDebug() << "Unfold posts:" << startIndex;

    if (startIndex < 0 || startIndex >= (int)mFeed.size())
    {
        qWarning() << "Invalid index:" << startIndex << "size:" << mFeed.size();
        return;
    }

    for (int i = startIndex; i < (int)mFeed.size(); ++i)
    {
        auto& post = mFeed[i];

        if (post.getFoldedPostType() == QEnums::FOLDED_POST_NONE)
            break;

        post.setFoldedPostType(QEnums::FOLDED_POST_NONE);
    }

    changeData({ int(Role::PostFoldedType) });
}

int AbstractPostFeedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mFeed.size();
}

QVariant AbstractPostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mFeed.size())
        return {};

    const auto& post = mFeed[index.row()];
    const auto* change = getLocalChange(post.getCid());

    switch (Role(role))
    {
    case Role::Author:
    {
        const auto author = post.getAuthor();
        const BasicProfile* profileChange = getProfileChange(author.getDid());
        return QVariant::fromValue(profileChange ? *profileChange : author);
    }
    case Role::PostText:
        return post.getFormattedText(mFocusHashtags.getNormalizedMatchHashtags(post));
    case Role::PostPlainText:
        return post.getText();
    case Role::PostLanguages:
        return QVariant::fromValue(post.getLanguages());
    case Role::PostUri:
        return post.getUri();
    case Role::PostCid:
        return post.getCid();
    case Role::PostIndexedDateTime:
        return post.getIndexedAt();
    case Role::PostIndexedSecondsAgo:
        return double((QDateTime::currentDateTimeUtc() - post.getIndexedAt()) / 1s);
    case Role::PostImages:
        return QVariant::fromValue(post.getImages());
    case Role::PostVideo:
    {
        auto video = post.getVideoView();
        return video ? QVariant::fromValue(*video) : QVariant{};
    }
    case Role::PostExternal:
    {
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant{};
    }
    case Role::PostRepostedByAuthor:
    {
        const auto& repostedBy = post.getRepostedBy();

        if (!repostedBy)
            return QVariant::fromValue(BasicProfile{});

        const BasicProfile* profileChange = getProfileChange(repostedBy->getDid());
        return QVariant::fromValue(profileChange ? *profileChange : *repostedBy);
    }
    case Role::PostRecord:
    {
        auto postRecord = post.getRecordView();
        RecordView* record = postRecord.get();

        if (!record)
            return QVariant{};

        if (change)
        {
            if (change->mDetachedRecord)
                record = change->mDetachedRecord.get();
            else if (change->mReAttachedRecord)
                record = change->mReAttachedRecord.get();
        }

        if (record->isReply())
        {
            const QString did = record->getReplyToAuthorDid();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                AuthorCache::instance().putProfile(did);
        }

        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record->getLabelsIncludingAuthorLabels());
        record->setContentVisibility(visibility);
        record->setContentWarning(warning);
        record->setMutedReason(mMutedWords);
        return QVariant::fromValue(*record);
    }
    case Role::PostRecordWithMedia:
    {
        auto recordWithMedia = post.getRecordWithMediaView();

        if (!recordWithMedia)
            return QVariant();

        if (change)
        {
            if (change->mDetachedRecord)
                recordWithMedia->setRecord(change->mDetachedRecord);
            else if (change->mReAttachedRecord)
                recordWithMedia->setRecord(change->mReAttachedRecord);
        }

        auto& record = recordWithMedia->getRecord();

        if (record.isReply())
        {
            const QString did = record.getReplyToAuthorDid();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                AuthorCache::instance().putProfile(did);
        }

        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record.getLabelsIncludingAuthorLabels());
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setMutedReason(mMutedWords);
        return QVariant::fromValue(*recordWithMedia);
    }
    case Role::PostType:
        return post.getPostType();
    case Role::PostFoldedType:
        return post.getFoldedPostType();
    case Role::PostThreadType:
        return post.getThreadType();
    case Role::PostIsPlaceHolder:
        return post.isPlaceHolder();
    case Role::PostGapId:
        return post.getGapId();
    case Role::PostHiddenPosts:
        return post.isHiddenPosts();
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
        if (!post.isReply())
            return {};

        const auto author = post.getReplyToAuthor();

        if (!author)
        {
            const QString did = post.getReplyToAuthorDid();

            if (!did.isEmpty())
                AuthorCache::instance().putProfile(did);

            return {};
        }

        const BasicProfile* profileChange = getProfileChange(author->getDid());
        return QVariant::fromValue(profileChange ? *profileChange : *author);
    }
    case Role::PostReplyRootAuthorDid:
        return post.getReplyToAuthorDid();
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
    case Role::PostQuoteCount:
        return post.getQuoteCount() + (change ? change->mQuoteCountDelta : 0);
    case Role::PostRepostUri:
        return change && change->mRepostUri ? *change->mRepostUri : post.getRepostUri();
    case Role::PostLikeUri:
        return change && change->mLikeUri ? *change->mLikeUri : post.getLikeUri();
    case Role::PostLikeTransient:
        return change ? change->mLikeTransient : false;
    case Role::PostThreadMuted:
    {
        const auto* uriChange = getLocalUriChange(post.isReply() ? post.getReplyRootUri() : post.getUri());
        return uriChange && uriChange->mThreadMuted ? *uriChange->mThreadMuted : post.isThreadMuted();
    }
    case Role::PostReplyDisabled:
        return post.isReplyDisabled();
    case Role::PostEmbeddingDisabled:
        return post.isEmbeddingDisabled();
    case Role::PostViewerStatePinned:
        return change && change->mViewerStatePinned ? *change->mViewerStatePinned : post.isViewerStatePinned();
    case Role::PostThreadgateUri:
    {
        if (!post.isReply())
            return change && change->mThreadgateUri ? *change->mThreadgateUri : post.getThreadgateUri();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mThreadgateUri ? *rootChange->mThreadgateUri : post.getThreadgateUri();
    }
    case Role::PostReplyRestriction:
    {
        if (!post.isReply())
            return change && change->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? change->mReplyRestriction : post.getReplyRestriction();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? rootChange->mReplyRestriction : post.getReplyRestriction();
    }
    case Role::PostReplyRestrictionLists:
    {
        if (!post.isReply())
            return QVariant::fromValue(change && change->mReplyRestrictionLists ? *change->mReplyRestrictionLists : post.getReplyRestrictionLists());

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return QVariant::fromValue(rootChange && rootChange->mReplyRestrictionLists ? *rootChange->mReplyRestrictionLists : post.getReplyRestrictionLists());
    }
    case Role::PostHiddenReplies:
    {
        if (!post.isReply())
            return change && change->mHiddenReplies ? *change->mHiddenReplies : post.getHiddenReplies();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mHiddenReplies ? *rootChange->mHiddenReplies : post.getHiddenReplies();
    }
    case Role::PostIsHiddenReply:
    {
        if (!post.isReply())
            return change && change->mHiddenReplies ? change->mHiddenReplies->contains(post.getUri()) : post.isHiddenReply();

        const QString rootCid = post.getReplyRootCid();
        const auto* rootChange = !rootCid.isEmpty() ? getLocalChange(rootCid) : nullptr;
        return rootChange && rootChange->mHiddenReplies ? rootChange->mHiddenReplies->contains(post.getUri()) : post.isHiddenReply();
    }
    case Role::PostBookmarked:
        return mBookmarks.isBookmarked(post.getUri());
    case Role::PostBookmarkNotFound:
        return post.isBookmarkNotFound();
    case Role::PostLabels:
        return QVariant::fromValue(ContentFilter::getContentLabels(post.getLabels()));
    case Role::PostContentVisibility:
    {
        const auto [visibility, _] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels());
        return visibility;
    }
    case Role::PostContentWarning:
    {
        const auto [_, warning] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels());
        return warning;
    }
    case Role::PostMutedReason:
    {
        if (post.getAuthor().getViewer().isMuted())
            return QEnums::MUTED_POST_AUTHOR;

        if (mMutedWords.match(post))
            return QEnums::MUTED_POST_WORDS;

        return QEnums::MUTED_POST_NONE;
    }
    case Role::PostHighlightColor:
    {
        const QColor color = mFocusHashtags.highlightColor(post);
        return color.isValid() ? color.name() : "transparent";
    }
    case Role::PostIsPinned:
        return post.isPinned();
    case Role::PostLocallyDeleted:
    {
        if (!change)
            return false;

        if (change->mPostDeleted)
            return true;

        if (!change->mRepostUri)
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
        { int(Role::PostPlainText), "postPlainText" },
        { int(Role::PostLanguages), "postLanguages" },
        { int(Role::PostIndexedDateTime), "postIndexedDateTime" },
        { int(Role::PostIndexedSecondsAgo), "postIndexedSecondsAgo" },
        { int(Role::PostRepostedByAuthor), "postRepostedByAuthor" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostVideo), "postVideo" },
        { int(Role::PostExternal), "postExternal" },
        { int(Role::PostRecord), "postRecord" },
        { int(Role::PostRecordWithMedia), "postRecordWithMedia" },
        { int(Role::PostType), "postType" },
        { int(Role::PostFoldedType), "postFoldedType" },
        { int(Role::PostThreadType), "postThreadType" },
        { int(Role::PostIsPlaceHolder), "postIsPlaceHolder" },
        { int(Role::PostGapId), "postGapId" },
        { int(Role::PostHiddenPosts), "postHiddenPosts" },
        { int(Role::PostNotFound), "postNotFound" },
        { int(Role::PostBlocked), "postBlocked" },
        { int(Role::PostNotSupported), "postNotSupported" },
        { int(Role::PostUnsupportedType), "postUnsupportedType" },
        { int(Role::PostIsReply), "postIsReply" },
        { int(Role::PostParentInThread), "postParentInThread" },
        { int(Role::PostReplyToAuthor), "postReplyToAuthor" },
        { int(Role::PostReplyRootAuthorDid), "postReplyRootAuthorDid" },
        { int(Role::PostReplyRootUri), "postReplyRootUri" },
        { int(Role::PostReplyRootCid), "postReplyRootCid" },
        { int(Role::PostReplyCount), "postReplyCount" },
        { int(Role::PostRepostCount), "postRepostCount" },
        { int(Role::PostLikeCount), "postLikeCount" },
        { int(Role::PostQuoteCount), "postQuoteCount" },
        { int(Role::PostRepostUri), "postRepostUri" },
        { int(Role::PostLikeUri), "postLikeUri" },
        { int(Role::PostLikeTransient), "postLikeTransient" },
        { int(Role::PostThreadMuted), "postThreadMuted" },
        { int(Role::PostReplyDisabled), "postReplyDisabled" },
        { int(Role::PostEmbeddingDisabled), "postEmbeddingDisabled" },
        { int(Role::PostViewerStatePinned), "postViewerStatePinned" },
        { int(Role::PostThreadgateUri), "postThreadgateUri" },
        { int(Role::PostReplyRestriction), "postReplyRestriction" },
        { int(Role::PostReplyRestrictionLists), "postReplyRestrictionLists" },
        { int(Role::PostHiddenReplies), "postHiddenReplies" },
        { int(Role::PostIsHiddenReply), "postIsHiddenReply" },
        { int(Role::PostBookmarked), "postBookmarked" },
        { int(Role::PostBookmarkNotFound), "postBookmarkNotFound" },
        { int(Role::PostLabels), "postLabels" },
        { int(Role::PostContentVisibility), "postContentVisibility" },
        { int(Role::PostContentWarning), "postContentWarning" },
        { int(Role::PostMutedReason), "postMutedReason" },
        { int(Role::PostHighlightColor), "postHighlightColor" },
        { int(Role::PostIsPinned), "postIsPinned" },
        { int(Role::PostLocallyDeleted), "postLocallyDeleted" },
        { int(Role::EndOfFeed), "endOfFeed" }
    };

    return roles;
}

void AbstractPostFeedModel::postIndexedSecondsAgoChanged()
{
    changeData({ int(Role::PostIndexedSecondsAgo) });
}

// For a change on a single post a single row change would be sufficient.
// However the model may have changed while waiting for a confirmation from the
// network about the change, so we don't know which row this CID is anymore.
// Also for reposts a CID may apply to multiple rows. Therefor all rows
// are marked as change. Performance seems not an issue.
void AbstractPostFeedModel::likeCountChanged()
{
    changeData({ int(Role::PostLikeCount) });
}

void AbstractPostFeedModel::likeUriChanged()
{
    changeData({ int(Role::PostLikeUri) });
}

void AbstractPostFeedModel::likeTransientChanged()
{
    changeData({ int(Role::PostLikeTransient) });
}

void AbstractPostFeedModel::replyCountChanged()
{
    changeData({ int(Role::PostReplyCount) });
}

void AbstractPostFeedModel::repostCountChanged()
{
    changeData({ int(Role::PostRepostCount) });
}

void AbstractPostFeedModel::quoteCountChanged()
{
    changeData({ int(Role::PostQuoteCount) });
}

void AbstractPostFeedModel::repostUriChanged()
{
    changeData({ int(Role::PostRepostUri), int(Role::PostLocallyDeleted) });
}

void AbstractPostFeedModel::threadgateUriChanged()
{
    changeData({ int(Role::PostThreadgateUri) });
}

void AbstractPostFeedModel::replyRestrictionChanged()
{
    changeData({ int(Role::PostReplyRestriction) });
}

void AbstractPostFeedModel::replyRestrictionListsChanged()
{
    changeData({ int(Role::PostReplyRestrictionLists) });
}

void AbstractPostFeedModel::hiddenRepliesChanged()
{
    changeData({ int(Role::PostHiddenReplies), int(Role::PostIsHiddenReply) });
}

void AbstractPostFeedModel::threadMutedChanged()
{
    changeData({ int(Role::PostThreadMuted) });
}

void AbstractPostFeedModel::detachedRecordChanged()
{
    changeData({ int(Role::PostRecord), int(Role::PostRecordWithMedia) });
}

void AbstractPostFeedModel::reAttachedRecordChanged()
{
    changeData({ int(Role::PostRecord), int(Role::PostRecordWithMedia) });
}

void AbstractPostFeedModel::viewerStatePinnedChanged()
{
    changeData({ int(Role::PostViewerStatePinned) });
}

void AbstractPostFeedModel::postDeletedChanged()
{
    changeData({ int(Role::PostLocallyDeleted) });
}

void AbstractPostFeedModel::profileChanged()
{
    changeData({ int(Role::Author), int(Role::PostReplyToAuthor), int(Role::PostRepostedByAuthor) });
}

void AbstractPostFeedModel::postBookmarkedChanged()
{
    changeData({ int(Role::PostBookmarked) });
}

void AbstractPostFeedModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mFeed.size() - 1, 0), roles);
}

}
