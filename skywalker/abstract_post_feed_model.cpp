// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "abstract_post_feed_model.h"
#include "author_cache.h"
#include "content_filter.h"
#include "content_filter_stats_model.h"
#include "focus_hashtags.h"
#include "list_cache.h"
#include "list_store.h"
#include "post_thread_cache.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

using namespace std::chrono_literals;

const QString AbstractPostFeedModel::NULL_STRING;
const ProfileStore AbstractPostFeedModel::NULL_PROFILE_STORE;
const ListStore AbstractPostFeedModel::NULL_LIST_STORE;
const ContentFilterShowAll AbstractPostFeedModel::NULL_CONTENT_FILTER;
const MutedWordsNoMutes AbstractPostFeedModel::NULL_MATCH_WORDS;
const FocusHashtags AbstractPostFeedModel::NULL_FOCUS_HASHTAGS;
HashtagIndex AbstractPostFeedModel::NULL_HASHTAG_INDEX{0};

AbstractPostFeedModel::AbstractPostFeedModel(QObject* parent) :
    QAbstractListModel(parent),
    mUserDid{NULL_STRING},
    mMutedReposts(NULL_PROFILE_STORE),
    mFeedHide(NULL_LIST_STORE),
    mContentFilter(NULL_CONTENT_FILTER),
    mMutedWords(NULL_MATCH_WORDS),
    mFocusHashtags(NULL_FOCUS_HASHTAGS),
    mHashtags(NULL_HASHTAG_INDEX)
{
}

AbstractPostFeedModel::AbstractPostFeedModel(const QString& userDid,
                                             const IProfileStore& mutedReposts,
                                             const IListStore& feedHide,
                                             const IContentFilter& contentFilter,
                                             const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid),
    mMutedReposts(mutedReposts),
    mFeedHide(feedHide),
    mContentFilter(contentFilter),
    mMutedWords(mutedWords),
    mFocusHashtags(focusHashtags),
    mHashtags(hashtags)
{
    connect(&AuthorCache::instance(), &AuthorCache::profileAdded, this,
            [this](const QString& did) {
                authorAdded(did);
                labelerAdded(did);
            },
            Qt::QueuedConnection);

    connect(&PostThreadCache::instance(), &PostThreadCache::postAdded, this,
            [this](const QString& uri){ postIsThreadChanged(uri); }, Qt::QueuedConnection);

    connect(&ListCache::instance(), &ListCache::listAdded, this,
            [this](const QString& uri){ listAdded(uri); }, Qt::QueuedConnection);
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

int AbstractPostFeedModel::toPhysicalIndex(int visibleIndex) const
{
    if (!mReverseFeed)
        return visibleIndex;

    return mFeed.size() - 1 - visibleIndex;
}

int AbstractPostFeedModel::toVisibleIndex(int physicalIndex, std::optional<int> feedSize) const
{
    if (!mReverseFeed)
        return physicalIndex;

    if (!feedSize)
        feedSize = mFeed.size();

    return *feedSize - 1 - physicalIndex;
}

void AbstractPostFeedModel::clearFeed()
{
    mFeed.clear();
    mStoredCids.clear();
    mStoredCidQueue = {};
    mContentFilterStats.clear();
    setEndOfFeed(false);
    clearLocalChanges();
    clearLocalProfileChanges();
}

void AbstractPostFeedModel::deletePost(int index)
{
    mFeed.erase(mFeed.begin() + toPhysicalIndex(index));
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

std::pair<QEnums::HideReasonType, ContentFilterStats::Details> AbstractPostFeedModel::mustHideContent(const Post& post) const
{
    const auto& author = post.getAuthor();
    const auto repostedBy = post.getRepostedBy();

    if (author.getViewer().isMuted())
    {
        qDebug() << "Hide post of muted author:" << author.getHandleOrDid() << post.getCid();
        return { QEnums::HIDE_REASON_MUTED_AUTHOR, author };
    }

    if (repostedBy && repostedBy->getViewer().isMuted())
    {
        qDebug() << "Hide repost of muted author:" << repostedBy->getHandleOrDid() << post.getCid();
        return { QEnums::HIDE_REASON_MUTED_AUTHOR, *repostedBy };
    }

    if (mFeedHide.contains(author.getDid()))
    {
        qDebug () << "Hide post from author:" << author.getHandleOrDid();
        return { QEnums::HIDE_REASON_HIDE_FROM_FOLLOWING_FEED, author };
    }

    if (repostedBy && mFeedHide.contains(repostedBy->getDid()))
    {
        qDebug () << "Hide repost from author:" << repostedBy->getHandleOrDid();
        return { QEnums::HIDE_REASON_HIDE_FROM_FOLLOWING_FEED, *repostedBy };
    }

    const auto& postLabels = post.getLabelsIncludingAuthorLabels();
    const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
        author, postLabels, mOverrideAdultVisibility);

    if (visibility == QEnums::CONTENT_VISIBILITY_HIDE_POST)
    {
        qDebug() << "Hide post:" << post.getCid() << warning;

        if (labelIndex >= 0 && labelIndex < postLabels.size())
            return { QEnums::HIDE_REASON_LABEL, postLabels[labelIndex] };
        else
            return { QEnums::HIDE_REASON_LABEL, nullptr };
    }

    if (post.isRepost() && mMutedReposts.contains(post.getRepostedBy()->getDid()))
    {
        qDebug() << "Mute repost, did:" << post.getRepostedBy()->getDid();
        return { QEnums::HIDE_REASON_REPOST_FROM_AUTHOR, *post.getRepostedBy() };
    }

    if (auto match = mMutedWords.match(post); match.first)
    {
        qDebug() << "Hide post due to muted words" << post.getCid();
        return { QEnums::HIDE_REASON_MUTED_WORD, MutedWordEntry(match.second) };
    }

    const auto& record = post.getRecordViewFromRecordOrRecordWithMedia();

    if (record)
    {
        if (auto match = mMutedWords.match(*record); match.first)
        {
            qDebug() << "Hide post due to muted words in record" << post.getCid();
            return { QEnums::HIDE_REASON_MUTED_WORD, MutedWordEntry(match.second) };
        }
    }

    return { QEnums::HIDE_REASON_NONE, nullptr };
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

const Post& AbstractPostFeedModel::getPost(int index) const
{
    static const Post NULL_POST;

    if (index < 0 || index >= (int)mFeed.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mFeed.size() << "modelId:" << mModelId;
        return NULL_POST;
    }

    return mFeed.at(toPhysicalIndex(index));
}

void AbstractPostFeedModel::unfoldPosts(int startIndex)
{
    qDebug() << "Unfold posts:" << startIndex;

    if (startIndex < 0 || startIndex >= (int)mFeed.size())
    {
        qWarning() << "Invalid index:" << startIndex << "size:" << mFeed.size();
        return;
    }

    // TODO: works only in reverse if post threads are reverted
    for (int i = toPhysicalIndex(startIndex); i < (int)mFeed.size(); ++i)
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
    if (mFeed.empty())
        return {};

    return mFeed.back().getTimelineTimestamp();
}

int AbstractPostFeedModel::findTimestamp(QDateTime timestamp, const QString& cid) const
{
    qDebug() << "Find timestamp:" << timestamp << "cid:" << cid;
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
            return toVisibleIndex(i);
    }

    return -1;
}

QDateTime AbstractPostFeedModel::getPostTimelineTimestamp(int index) const
{
    if (index < 0 || index >= (int)mFeed.size())
        return {};

    return mFeed[toPhysicalIndex(index)].getTimelineTimestamp();
}

QString AbstractPostFeedModel::getPostCid(int index) const
{
    if (index < 0 || index >= (int)mFeed.size())
        return {};

    return mFeed[toPhysicalIndex(index)].getCid();
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

void AbstractPostFeedModel::reset()
{
    beginResetModel();
    endResetModel();
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

    const int physicalIndex = toPhysicalIndex(index.row());
    const auto& post = mFeed[physicalIndex];
    const auto* change = getLocalChange(post.getCid());

    switch (Role(role))
    {
    case Role::UserDid:
        return mUserDid;
    case Role::Author:
    {
        const auto author = !post.isBlocked() ? post.getAuthor() : post.getBlockedAuthor().getAuthor();
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
    case Role::PostReasonRepostUri:
        return post.getReasonRepostUri();
    case Role::PostReasonRepostCid:
        return post.getReasonRepostCid();
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

        if (record->getBlocked())
        {
            const auto& blockedAuthor = record->getBlockedAuthor();
            const QString did = blockedAuthor.getDid();
            const QString listUri = blockedAuthor.getBlockingByListUri();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });

            if (!listUri.isEmpty() && blockedAuthor.getBlockingByList().isNull())
                QTimer::singleShot(0, this, [listUri]{ ListCache::instance().putList(listUri); });
        }

        const auto& recordLabels = record->getLabelsIncludingAuthorLabels();
        auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
            record->getAuthor(), recordLabels, mOverrideAdultVisibility);
        record->setContentVisibility(visibility);
        record->setContentWarning(warning);
        record->setContentLabeler(getContentLabeler(visibility, recordLabels, labelIndex));
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

        if (record.getBlocked())
        {
            const auto& blockedAuthor = record.getBlockedAuthor();
            const QString did = blockedAuthor.getDid();
            const QString listUri = blockedAuthor.getBlockingByListUri();

            if (!did.isEmpty() && !AuthorCache::instance().contains(did))
                QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });

            if (!listUri.isEmpty() && blockedAuthor.getBlockingByList().isNull())
                QTimer::singleShot(0, this, [listUri]{ ListCache::instance().putList(listUri); });
        }

        const auto& recordLabels = record.getLabelsIncludingAuthorLabels();
        const auto [visibility, warning, labelIndex] = mContentFilter.getVisibilityAndWarning(
            record.getAuthor(), recordLabels, mOverrideAdultVisibility);
        record.setContentVisibility(visibility);
        record.setContentWarning(warning);
        record.setContentLabeler(getContentLabeler(visibility, recordLabels, labelIndex));
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
    case Role::PostBlockedAuthor:
    {
        const auto& blockedAuthor = post.getBlockedAuthor();
        const QString did = blockedAuthor.getDid();
        const QString listUri = blockedAuthor.getBlockingByListUri();

        if (!did.isEmpty() && !AuthorCache::instance().contains(did))
            QTimer::singleShot(0, this, [did]{ AuthorCache::instance().putProfile(did); });

        if (!listUri.isEmpty() && blockedAuthor.getBlockingByList().isNull())
            QTimer::singleShot(0, this, [listUri]{ ListCache::instance().putList(listUri); });

        return QVariant::fromValue(blockedAuthor);
    }
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
    case Role::PostMentionDids:
        return post.getMentionDids();
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
        return change && change->mBookmarked ? *change->mBookmarked : post.isBookmarked();
    case Role::PostBookmarkTransient:
        return change ? change->mBookmarkTransient : false;
    case Role::PostFeedback:
        return change ? change->mFeedback : QEnums::FEEDBACK_NONE;
    case Role::PostFeedbackTransient:
        return change ? change->mFeedbackTransient : QEnums::FEEDBACK_NONE;
    case Role::PostLabels:
        return QVariant::fromValue(ContentFilter::getContentLabels(post.getLabels()));
    case Role::PostContentVisibility:
    {
        const auto [visibility, _, __] = mContentFilter.getVisibilityAndWarning(
            post.getAuthor(), post.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
        return visibility;
    }
    case Role::PostContentWarning:
    {
        const auto [_, warning, __] = mContentFilter.getVisibilityAndWarning(
            post.getAuthor(), post.getLabelsIncludingAuthorLabels(), mOverrideAdultVisibility);
        return warning;
    }
    case Role::PostContentLabeler:
    {
        const auto& labels = post.getLabelsIncludingAuthorLabels();
        const auto [visibility, _, labelIndex] = mContentFilter.getVisibilityAndWarning(
            post.getAuthor(), labels, mOverrideAdultVisibility);
        const auto labeler = getContentLabeler(visibility, labels, labelIndex);
        return QVariant::fromValue(labeler);
    }
    case Role::PostMutedReason:
    {
        if (post.getAuthor().getViewer().isMuted())
            return QEnums::MUTED_POST_AUTHOR;

        if (mMutedWords.match(post).first)
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
    case Role::PostIsThreadReply:
        return post.isThreadReply();
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
    case Role::FilteredPostHideReason:
        return QEnums::HIDE_REASON_NONE;
    case Role::FilteredPostHideDetail:
        return "";
    case Role::FilteredPostContentLabel:
        return QVariant::fromValue(ContentLabel{});
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
        { int(Role::UserDid), "userDid" },
        { int(Role::Author), "author" },
        { int(Role::PostUri), "postUri" },
        { int(Role::PostCid), "postCid" },
        { int(Role::PostText), "postText" },
        { int(Role::PostPlainText), "postPlainText" },
        { int(Role::PostLanguages), "postLanguages" },
        { int(Role::PostIndexedDateTime), "postIndexedDateTime" },
        { int(Role::PostIndexedSecondsAgo), "postIndexedSecondsAgo" },
        { int(Role::PostRepostedByAuthor), "postRepostedByAuthor" },
        { int(Role::PostReasonRepostUri), "postReasonRepostUri" },
        { int(Role::PostReasonRepostCid), "postReasonRepostCid" },
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
        { int(Role::PostBlockedAuthor), "postBlockedAuthor" },
        { int(Role::PostNotSupported), "postNotSupported" },
        { int(Role::PostUnsupportedType), "postUnsupportedType" },
        { int(Role::PostIsReply), "postIsReply" },
        { int(Role::PostParentInThread), "postParentInThread" },
        { int(Role::PostReplyToAuthor), "postReplyToAuthor" },
        { int(Role::PostReplyRootAuthorDid), "postReplyRootAuthorDid" },
        { int(Role::PostReplyRootUri), "postReplyRootUri" },
        { int(Role::PostReplyRootCid), "postReplyRootCid" },
        { int(Role::PostMentionDids), "postMentionDids" },
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
        { int(Role::PostBookmarkTransient), "postBookmarkTransient" },
        { int(Role::PostFeedback), "postFeedback" },
        { int(Role::PostFeedbackTransient), "postFeedbackTransient" },
        { int(Role::PostLabels), "postLabels" },
        { int(Role::PostContentVisibility), "postContentVisibility" },
        { int(Role::PostContentWarning), "postContentWarning" },
        { int(Role::PostContentLabeler), "postContentLabeler" },
        { int(Role::PostMutedReason), "postMutedReason" },
        { int(Role::PostHighlightColor), "postHighlightColor" },
        { int(Role::PostIsPinned), "postIsPinned" },
        { int(Role::PostIsThread), "postIsThread" },
        { int(Role::PostIsThreadReply), "postIsThreadReply" },
        { int(Role::PostLocallyDeleted), "postLocallyDeleted" },
        { int(Role::FilteredPostHideReason), "filteredPostHideReason" },
        { int(Role::FilteredPostHideDetail), "filteredPostHideDetail" },
        { int(Role::FilteredPostContentLabel), "filteredPostContentLabel" },
        { int(Role::EndOfFeed), "endOfFeed" }
    };

    return roles;
}

void AbstractPostFeedModel::beginRemoveRowsPhysical(int firstPhysicalIndex, int lastPhysicalIndex)
{
    if (!mReverseFeed)
        beginRemoveRows({}, firstPhysicalIndex, lastPhysicalIndex);
    else
        beginRemoveRows({}, toVisibleIndex(lastPhysicalIndex), toVisibleIndex(firstPhysicalIndex));
}

void AbstractPostFeedModel::beginInsertRowsPhysical(int firstPhysicalIndex, int lastPhysicalIndex)
{
    if (!mReverseFeed)
    {
        beginInsertRows({}, firstPhysicalIndex, lastPhysicalIndex);
    }
    else
    {
        const int newFeedSize = mFeed.size() + (lastPhysicalIndex - firstPhysicalIndex) + 1;
        beginInsertRows({}, toVisibleIndex(lastPhysicalIndex, newFeedSize),
                        toVisibleIndex(firstPhysicalIndex, newFeedSize));
    }
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

void AbstractPostFeedModel::bookmarkedChanged()
{
    changeData({ int(Role::PostBookmarked) });
}

void AbstractPostFeedModel::bookmarkTransientChanged()
{
    changeData({ int(Role::PostBookmarkTransient) });
}

void AbstractPostFeedModel::feedbackChanged()
{
    changeData({ int(Role::PostFeedback) });
}

void AbstractPostFeedModel::feedbackTransientChanged()
{
    changeData({ int(Role::PostFeedbackTransient) });
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

void AbstractPostFeedModel::authorAdded(const QString& did)
{
    for (int i = 0; i < (int)mFeed.size(); ++i)
    {
        const auto& post = mFeed[i];

        if (post.getReplyToAuthorDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostReplyToAuthor) });

        if (post.getBlockedAuthor().getDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostBlockedAuthor), int(Role::Author) });

        const auto postRecord = post.getRecordView();

        if (postRecord && postRecord->getReplyToAuthorDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecord) });

        if (postRecord && postRecord->getBlockedAuthor().getDid() == did)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecord) });

        const auto recordWithMedia = post.getRecordWithMediaView();

        if (recordWithMedia)
        {
            const auto record = recordWithMedia->getRecordPtr();

            if (record && record->getReplyToAuthorDid() == did)
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecordWithMedia) });

            if (record && record->getBlockedAuthor().getDid() == did)
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecordWithMedia) });
        }
    }
}

void AbstractPostFeedModel::labelerAdded(const QString& did)
{
    const auto* profile = AuthorCache::instance().get(did);

    if (profile && profile->getAssociated().isLabeler())
        changeData({ int(Role::PostContentLabeler), int(Role::PostRecord), int(Role::PostRecordWithMedia) });
}

void AbstractPostFeedModel::listAdded(const QString& uri)
{
    for (int i = 0; i < (int)mFeed.size(); ++i)
    {
        const auto& post = mFeed[i];

        if (post.getBlockedAuthor().getBlockingByListUri() == uri)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostBlockedAuthor) });

        const auto postRecord = post.getRecordView();

        if (postRecord && postRecord->getBlockedAuthor().getBlockingByListUri() == uri)
            emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecord) });

        const auto recordWithMedia = post.getRecordWithMediaView();

        if (recordWithMedia)
        {
            const auto record = recordWithMedia->getRecordPtr();

            if (record && record->getBlockedAuthor().getBlockingByListUri() == uri)
                emit dataChanged(createIndex(i, 0), createIndex(i, 0), { int(Role::PostRecordWithMedia) });
        }
    }
}

BasicProfile AbstractPostFeedModel::getContentLabeler(QEnums::ContentVisibility visibility,
                                                      const ContentLabelList& labels,
                                                      int labelIndex) const
{
    if (visibility == QEnums::CONTENT_VISIBILITY_SHOW)
        return {};

    if (labelIndex < 0 || labelIndex >= labels.size())
        return {};

    const QString& labelerDid = labels[labelIndex].getDid();

    if (labelerDid.isEmpty())
        return {};

    const BasicProfile* profile = AuthorCache::instance().get(labelerDid);

    if (profile)
        return *profile;

    QTimer::singleShot(0, this, [labelerDid]{ AuthorCache::instance().putProfile(labelerDid); });
    return {};
}

ContentFilterStatsModel* AbstractPostFeedModel::createContentFilterStatsModel()
{
    qDebug() << "Feed size:" << mFeed.size() << "checked:" << mContentFilterStats.checkedPosts() << "filtered:" << mContentFilterStats.total();
    auto* model = new ContentFilterStatsModel(mContentFilterStats, mContentFilter, this);
    return model;
}

}
