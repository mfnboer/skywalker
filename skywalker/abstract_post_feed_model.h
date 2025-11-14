// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "content_filter.h"
#include "content_filter_stats.h"
#include "hashtag_index.h"
#include "local_post_model_changes.h"
#include "local_profile_changes.h"
#include "muted_words.h"
#include "post.h"
#include "profile_store.h"
#include <QAbstractListModel>
#include <deque>
#include <queue>
#include <unordered_set>

namespace Skywalker {

class ContentFilterStatsModel;
class FocusHashtags;

class AbstractPostFeedModel : public QAbstractListModel,
                              public BaseListModel,
                              public LocalPostModelChanges,
                              public LocalProfileChanges
{
    Q_MOC_INCLUDE("content_filter_stats_model.h")

    Q_OBJECT
    Q_PROPERTY(bool endOfFeed READ isEndOfFeed NOTIFY endOfFeedChanged FINAL)
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(QString error READ getFeedError NOTIFY feedErrorChanged FINAL)
    QML_ELEMENT

public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;

    enum class Role {
        UserDid = Qt::UserRole + 1,
        Author,
        PostUri,
        PostCid,
        PostText, // Formatted
        PostPlainText,
        PostLanguages,
        PostIndexedDateTime,
        PostIndexedSecondsAgo,
        PostRepostedByAuthor,
        PostReasonRepostUri,
        PostReasonRepostCid,
        PostHasUnknownEmbed,
        PostUnknownEmbedType,
        PostImages,
        PostVideo,
        PostExternal,
        PostRecord,
        PostRecordWithMedia,
        PostType,
        PostFoldedType,
        PostThreadType,
        PostThreadIndentLevel,
        PostIsPlaceHolder,
        PostGapId,
        PostHiddenPosts,
        PostNotFound,
        PostBlocked,
        PostNotSupported,
        PostUnsupportedType,
        PostIsReply,
        PostParentInThread,
        PostReplyToAuthor,
        PostReplyRootAuthorDid,
        PostReplyRootUri,
        PostReplyRootCid,
        PostMentionDids,
        PostFeedContext,
        PostReplyCount,
        PostRepostCount,
        PostLikeCount,
        PostQuoteCount,
        PostRepostUri,
        PostLikeUri,
        PostLikeTransient,
        PostThreadMuted,
        PostReplyDisabled,
        PostEmbeddingDisabled,
        PostViewerStatePinned,
        PostThreadgateUri,
        PostReplyRestriction,
        PostReplyRestrictionLists,
        PostHiddenReplies,
        PostIsHiddenReply,
        PostBookmarked,
        PostBookmarkTransient,
        PostFeedback,
        PostFeedbackTransient,
        PostLabels,
        PostContentVisibility,
        PostContentWarning,
        PostContentLabeler,
        PostMutedReason,
        PostHighlightColor,
        PostIsPinned,
        PostIsThread,
        PostIsThreadReply,
        PostLocallyDeleted,
        FilteredPostHideReason,
        FilteredPostHideDetail,
        FilteredPostContentLabel,
        EndOfFeed
    };
    Q_ENUM(Role)

    using Ptr = std::unique_ptr<AbstractPostFeedModel>;

    explicit AbstractPostFeedModel(QObject* parent = nullptr);

    AbstractPostFeedModel(const QString& userDid, const IProfileStore& following,
                          const IProfileStore& mutedReposts,
                          const IProfileStore& feedHide,
                          const IContentFilter& contentFilter,
                          const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                          HashtagIndex& hashtags,
                          QObject* parent = nullptr);

    void setModelId(int modelId) { mModelId = modelId; }
    int getModelId() const { return mModelId; }

    void setOverrideAdultVisibility(const QEnums::ContentVisibility visibility) { mOverrideAdultVisibility = visibility; }
    void clearOverrideAdultVisibility() { mOverrideAdultVisibility = {}; }

    Q_INVOKABLE void setOverrideLinkColor(const QString& color);
    Q_INVOKABLE void clearOverrideLinkColor();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    virtual bool isEndOfFeed() const { return mEndOfFeed; }
    virtual void setEndOfFeed(bool endOfFeed);

    const Post& getPost(int index) const;
    Q_INVOKABLE void unfoldPosts(int startIndex);

    // Get the timestamp of the last post in the feed
    QDateTime lastTimestamp() const;

    // Returns the index of the last post >= timestamp, 0 if no such post exists
    // If  there are multiple posts with the same timestamp, then pick the one with
    // matching cid.
    Q_INVOKABLE int findTimestamp(QDateTime timestamp, const QString& cid) const;

    // Returns index of post, or -1 if post not found.
    int findPost(const QString& cid) const;

    Q_INVOKABLE QDateTime getPostTimelineTimestamp(int index) const;
    Q_INVOKABLE QString getPostCid(int index) const;

    virtual void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

    virtual void setFeedError(const QString& error);
    void clearFeedError() { setFeedError({}); }
    const QString& getFeedError() const { return mFeedError; }

    bool isFilteredPostFeed() const { return mPostHideInfoMap; }
    Q_INVOKABLE ContentFilterStatsModel* createContentFilterStatsModel();

signals:
    void endOfFeedChanged();
    void getFeedInProgressChanged();
    void feedErrorChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
    void clearFeed();
    void deletePost(int index);
    void storeCid(const QString& cid);
    void removeStoredCid(const QString& cid);
    void cleanupStoredCids();
    bool cidIsStored(const QString& cid) const { return mStoredCids.count(cid); }
    void preprocess(const Post& post);

    virtual std::pair<QEnums::HideReasonType, ContentFilterStats::Details> mustHideContent(const Post& post) const;

    // LocalPostModelChanges
    virtual void postIndexedSecondsAgoChanged() override;
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void likeTransientChanged() override;
    virtual void replyCountChanged() override;
    virtual void repostCountChanged() override;
    virtual void quoteCountChanged() override;
    virtual void repostUriChanged() override;
    virtual void threadgateUriChanged() override;
    virtual void replyRestrictionChanged() override;
    virtual void replyRestrictionListsChanged() override;
    virtual void hiddenRepliesChanged() override;
    virtual void threadMutedChanged() override;
    virtual void bookmarkedChanged() override;
    virtual void bookmarkTransientChanged() override;
    virtual void feedbackChanged() override;
    virtual void feedbackTransientChanged() override;
    virtual void detachedRecordChanged() override;
    virtual void reAttachedRecordChanged() override;
    virtual void viewerStatePinnedChanged() override;
    virtual void postDeletedChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;
    virtual void locallyBlockedChanged() override;

    void changeData(const QList<int>& roles) override;

    using TimelineFeed = std::deque<Post>;
    TimelineFeed mFeed;

    const QString& mUserDid;
    const IProfileStore& mFollowing;
    const IProfileStore& mMutedReposts;
    const IProfileStore& mFeedHide;
    const IContentFilter& mContentFilter;
    const IMatchWords& mMutedWords;
    const FocusHashtags& mFocusHashtags;
    HashtagIndex& mHashtags;
    ContentFilterStats mContentFilterStats;
    const ContentFilterStats::PostHideInfoMap* mPostHideInfoMap = nullptr;
    int mModelId = -1;

private:
    static const QString NULL_STRING;
    static const ProfileStore NULL_PROFILE_STORE;
    static const ContentFilterShowAll NULL_CONTENT_FILTER;
    static const MutedWordsNoMutes NULL_MATCH_WORDS;
    static const FocusHashtags NULL_FOCUS_HASHTAGS;
    static HashtagIndex NULL_HASHTAG_INDEX;

    void indexHashtags(const Post& post);
    void identifyThreadPost(const Post& post);

    void postIsThreadChanged(const QString& postUri);
    void replyToAuthorAdded(const QString& did);
    void labelerAdded(const QString& did);

    BasicProfile getContentLabeler(QEnums::ContentVisibility visibility,
                                   const ContentLabelList& labels,
                                   int labelIndex) const;

    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;
    std::optional<QEnums::ContentVisibility> mOverrideAdultVisibility;
    QString mOverrideLinkColor;
    bool mEndOfFeed = false;
    bool mGetFeedInProgress = false;
    QString mFeedError;
};

}
