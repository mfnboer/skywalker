// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "abstract_post_feed_model.h"
#include "author_cache.h"
#include "content_filter.h"
#include "focus_hashtags.h"
#include "muted_words.h"
#include "post_thread_cache.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

using namespace std::chrono_literals;

static const QString NULL_STRING;
static const ProfileStore NULL_PROFILE_STORE;
static const ContentFilterShowAll NULL_CONTENT_FILTER;
static const Bookmarks NULL_BOOKMARKS;
static const MutedWordsNoMutes NULL_MATCH_WORDS;
static const FocusHashtags NULL_FOCUS_HASHTAGS;
static HashtagIndex NULL_HASHTAG_INDEX(0);

AbstractPostFeedModel::AbstractPostFeedModel(QObject* parent) :
    QAbstractListModel(parent),
    mUserDid{NULL_STRING},
    mFollowing(NULL_PROFILE_STORE),
    mMutedReposts(NULL_PROFILE_STORE),
    mFeedHide(NULL_PROFILE_STORE),
    mContentFilter(NULL_CONTENT_FILTER),
    mBookmarks(NULL_BOOKMARKS),
    mMutedWords(NULL_MATCH_WORDS),
    mFocusHashtags(NULL_FOCUS_HASHTAGS),
    mHashtags(NULL_HASHTAG_INDEX)
{
}

AbstractPostFeedModel::AbstractPostFeedModel(const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IProfileStore& feedHide,
                                             const IContentFilter& contentFilter, const Bookmarks& bookmarks,
                                             const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid),
    mFollowing(following),
    mMutedReposts(mutedReposts),
    mFeedHide(feedHide),
    mContentFilter(contentFilter),
    mBookmarks(bookmarks),
    mMutedWords(mutedWords),
    mFocusHashtags(focusHashtags),
    mHashtags(hashtags)
{
    connect(&mBookmarks, &Bookmarks::sizeChanged, this, [this]{ postBookmarkedChanged(); }, Qt::QueuedConnection);
    connect(&AuthorCache::instance(), &AuthorCache::profileAdded, this,
            [this](const QString& did){ replyToAuthorAdded(did); }, Qt::QueuedConnection);
    connect(&PostThreadCache::instance(), &PostThreadCache::postAdded, this,
            [this](const QString& uri){ postIsThreadChanged(uri); }, Qt::QueuedConnection);
}

void AbstractPostFeedModel::setOverrideLinkColor(const QString& color)
{
    mOverrideLinkColor = color;
    changeData({ int(Role::PostText) });
}

void AbstractPostFeedModel::clearOverrideLinkColor()
{
    mOverrideLinkColor.clear();
    changeData({ int(Role::PostText) });
}

void AbstractPostFeedModel::clearFeed()
{
    mFeed.clear();
    mStoredCids.clear();
    mStoredCidQueue = {};
    setEndOfFeed(false);
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
    const auto& author = post.getAuthor();

    if (author.getViewer().isMuted())
    {
        qDebug() << "Hide post of muted author:" << author.getHandleOrDid() << post.getCid();
        return true;
    }

    if (mFeedHide.contains(author.getDid()))
    {
        qDebug () << "Hide post from author:" << author.getHandleOrDid();
        return true;
    }

    const auto repostedBy = post.getRepostedBy();
    if (repostedBy && mFeedHide.contains(repostedBy->getDid()))
    {
        qDebug () << "Hide repost from author:" << author.getHandleOrDid();
        return true;
    }

    const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);

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

    const auto& record = post.getRecordViewFromRecordOrRecordWithMedia();

    if (record && mMutedWords.match(*record))
    {
        qDebug() << "Hide post due to muted words in record" << post.getCid();
        return true;
    }

    return false;
}

void AbstractPostFeedModel::preprocess(const Post& post)
{
    indexHashtags(post);
    identifyThreadPost(post);
}

void AbstractPostFeedModel::indexHashtags(const Post& post)
{
    const auto hashtags = post.getHashtags();

    for (const auto& tag : hashtags)
        mHashtags.insert(tag);
}

void AbstractPostFeedModel::identifyThreadPost(const Post& post)
{
    const QString replyRootUri = post.getReplyRootUri();

    if (replyRootUri.isEmpty())
        return;

    if (post.getReplyToUri() == replyRootUri && post.getAuthorDid() == post.getReplyRootAuthorDid())
    {
        qDebug() << "Post is thread:" << replyRootUri;
        auto& postThreadCache = PostThreadCache::instance();
        postThreadCache.put(replyRootUri, true);
    }
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

QDateTime AbstractPostFeedModel::lastTimestamp() const
{
    return !mFeed.empty() ? mFeed.back().getTimelineTimestamp() : QDateTime();
}

int AbstractPostFeedModel::findTimestamp(QDateTime timestamp, const QString& cid) const
{
    int foundIndex = 0;

    for (int i = mFeed.size() - 1; i >= 0; --i)
    {
        const Post& post = mFeed[i];

        if (post.isPlaceHolder())
            continue;

        if (post.getTimelineTimestamp() == timestamp)
        {
            if (!cid.isEmpty() && post.getCid() == cid)
                return i;

            if (foundIndex == 0)
                foundIndex = i;
        }
        else if (post.getTimelineTimestamp() > timestamp)
        {
            return foundIndex > 0 ? foundIndex : i;
        }
    }

    return 0;
}

int AbstractPostFeedModel::findPost(const QString& cid) const
{
    for (int i = mFeed.size() - 1; i >= 0; --i)
    {
        const Post& post = mFeed[i];

        if (post.getCid() == cid)
            return i;
    }

    return -1;
}

QDateTime AbstractPostFeedModel::getPostTimelineTimestamp(int index) const
{
    if (index < 0 || index >= (int)mFeed.size())
        return {};

    return mFeed[index].getTimelineTimestamp();
}

QString AbstractPostFeedModel::getPostCid(int index) const
{
    if (index < 0 || index >= (int)mFeed.size())
        return {};

    return mFeed[index].getCid();
}

void AbstractPostFeedModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress)
    {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();

        if (mGetFeedInProgress)
            clearFeedError();
    }
}

void AbstractPostFeedModel::setFeedError(const QString& error)
{
    if (error != mFeedError)
    {
        mFeedError = error;
        emit feedErrorChanged();
    }
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
        return post.getFormattedText(mFocusHashtags.getNormalizedMatchHashtags(post), mOverrideLinkColor);
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
    case Role::PostHasUnknownEmbed:
        return post.hasUnknownEmbed();
    case Role::PostUnknownEmbedType:
        return post.getUnknownEmbedType();
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
                QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });
        }
        else
        {
            if (record->isThread() == QEnums::TRIPLE_BOOL_UNKNOWN)
                QTimer::singleShot(0, this, [postUri=record->getUri()]{ PostThreadCache::instance().putPost(postUri); });
        }

        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record->getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
        record->setContentVisibility(visibility);
        record->setContentWarning(warning);
        record->setMutedReason(mMutedWords);
        return QVariant::fromValue(*record);
    }
    case Role::PostRecordWithMedia:
    {
        auto recordWithMedia = post.getRecordWithMediaView();

        if (!recordWithMedia)
            return QVariant{};

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
                QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });
        }
        else
        {
            if (record.isThread() == QEnums::TRIPLE_BOOL_UNKNOWN)
                QTimer::singleShot(0, this, [postUri=record.getUri()]{ PostThreadCache::instance().putPost(postUri); });
        }

        const auto [visibility, warning] = mContentFilter.getVisibilityAndWarning(record.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
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
    case Role::PostThreadIndentLevel:
        return post.getThreadIndentLevel();
    case Role::PostIsPlaceHolder:
        return post.isPlaceHolder();
    case Role::PostGapId:
        return post.getGapId();
    case Role::PostHiddenPosts:
        return post.isHiddenPosts();
    case Role::PostNotFound:
        return post.isNotFound();
    case Role::PostBlocked:
        return post.isBlocked() || getLocallyBlocked(post.getAuthor().getDid());
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
                QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });

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
    case Role::PostFeedContext:
        return post.getFeedContext();
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
        const auto [visibility, _] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
        return visibility;
    }
    case Role::PostContentWarning:
    {
        const auto [_, warning] = mContentFilter.getVisibilityAndWarning(post.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
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
    case Role::PostIsThread:
    {
        const auto isThread = post.isThread();

        switch (isThread)
        {
        case QEnums::TRIPLE_BOOL_NO:
            return false;
        case QEnums::TRIPLE_BOOL_YES:
            return true;
        case QEnums::TRIPLE_BOOL_UNKNOWN:
            QTimer::singleShot(0, this, [postUri=post.getUri()]{ PostThreadCache::instance().putPost(postUri); });
            return false;
        }

        qWarning() << "Invalid isThread:" << isThread;
        return false;
    }
    case Role::PostLocallyDeleted:
    {
        if (getLocallyBlocked(post.getAuthor().getDid()))
            return true;

        if (!change)
            return false;

        if (change->mPostDeleted)
            return true;

        if (!change->mRepostUri)
            return false;

        auto repostedBy = post.getRepostedBy();
        if (!repostedBy)
            return false;

        if (getLocallyBlocked(repostedBy->getDid()))
            return true;

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

void AbstractPostFeedModel::setEndOfFeed(bool endOfFeed)
{
    if (endOfFeed != mEndOfFeed)
    {
        mEndOfFeed = endOfFeed;
        emit endOfFeedChanged();
    }
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
        { int(Role::PostHasUnknownEmbed), "postHasUnknownEmbed" },
        { int(Role::PostUnknownEmbedType), "postUnknownEmbedType" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostVideo), "postVideo" },
        { int(Role::PostExternal), "postExternal" },
        { int(Role::PostRecord), "postRecord" },
        { int(Role::PostRecordWithMedia), "postRecordWithMedia" },
        { int(Role::PostType), "postType" },
        { int(Role::PostFoldedType), "postFoldedType" },
        { int(Role::PostThreadType), "postThreadType" },
        { int(Role::PostThreadIndentLevel), "postThreadIndentLevel" },
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
        { int(Role::PostFeedContext), "postFeedContext" },
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
        { int(Role::PostIsThread), "postIsThread" },
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

void AbstractPostFeedModel::locallyBlockedChanged()
{
    changeData({ int(Role::PostBlocked), int(Role::PostLocallyDeleted) });
}

void AbstractPostFeedModel::postBookmarkedChanged()
{
    changeData({ int(Role::PostBookmarked) });
}

void AbstractPostFeedModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mFeed.size() - 1, 0), roles);
}

// Easier would be to do this:
//
// changeData({ int(Role::PostIsThread), int(Role::PostRecord), int(Role::PostRecordWithMedia) });
//
// However the code below is faster as it will not trigger a redraw of all posts.
void AbstractPostFeedModel::postIsThreadChanged(const QString& postUri)
{
    const bool* isThread = PostThreadCache::instance().getIsThread(postUri);

    // If the post is not a thread, then model assumed correctly it is not a thread,
    // so no change in the GUI.
    if (!isThread || *isThread == false)
        return;

    for (int i = 0; i < (int)mFeed.size(); ++i)
    {
        const auto& post = mFeed[i];

        if (post.getUri() == postUri)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostIsThread) });

        const auto postRecord = post.getRecordView();

        if (postRecord && postRecord->getUri() == postUri)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecord) });

        const auto recordWithMedia = post.getRecordWithMediaView();

        if (recordWithMedia)
        {
            const auto record = recordWithMedia->getRecordPtr();

            if (record && record->getUri() == postUri)
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecordWithMedia) });
        }
    }
}

void AbstractPostFeedModel::replyToAuthorAdded(const QString& did)
{
    for (int i = 0; i < (int)mFeed.size(); ++i)
    {
        const auto& post = mFeed[i];

        if (post.getReplyToAuthorDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostReplyToAuthor) });

        const auto postRecord = post.getRecordView();

        if (postRecord && postRecord->getReplyToAuthorDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecord) });

        const auto recordWithMedia = post.getRecordWithMediaView();

        if (recordWithMedia)
        {
            const auto record = recordWithMedia->getRecordPtr();

            if (record && record->getReplyToAuthorDid() == did)
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecordWithMedia) });
        }
    }
}

}
