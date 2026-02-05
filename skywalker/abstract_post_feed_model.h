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

using namespace std::chrono_literals;

class ContentFilterStatsModel;
class FocusHashtags;
class IListStore;

// Note on indexes.
// The feed posts are stored in mFeed in the natural feed order, e.g. new to old.
// The index in mFeed is the physical index.
// If the user reverses the feed view order, then mFeed stays *mostly*, the same. Only
// assembled post threads are reversed.
// The index into the the view (ListView) in the UI is the visible index.
// When the view order and then natural feed order are the same, then the visible index is
// the same is the physical index.
// When the view order is reversed, then visible index COUNT-1 is physical index 0 and v.v.

class AbstractPostFeedModel : public QAbstractListModel,
                              public BaseListModel,
                              public LocalPostModelChanges,
                              public LocalProfileChanges
{
    Q_MOC_INCLUDE("content_filter_stats_model.h")

    Q_OBJECT
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(bool reverseFeed READ isReverseFeed WRITE setReverseFeed NOTIFY reverseFeedChanged FINAL)
    Q_PROPERTY(bool endOfFeed READ isEndOfFeed NOTIFY endOfFeedChanged FINAL)
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(QString error READ getFeedError NOTIFY feedErrorChanged FINAL)
    Q_PROPERTY(bool chronological READ isChronological NOTIFY chronologicalChanged FINAL)
    QML_ELEMENT
    QML_UNCREATABLE("only subclasses can be created")

public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;

    // I have seen that posts in chronological feeds like timeline and list feeds are not
    // always strictly chronological. I have seen deviations up to 40s.
    static constexpr auto MAX_CHRONO_DEVIATION_DT = 50s;

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
        PostBlockedAuthor,
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

    AbstractPostFeedModel(const QString& userDid,
                          const IProfileStore& mutedReposts,
                          const IListStore& feedHide,
                          const IContentFilter& contentFilter,
                          const IMatchWords& mutedWords, const FocusHashtags& focusHashtags,
                          HashtagIndex& hashtags,
                          QObject* parent = nullptr);

    virtual ~AbstractPostFeedModel() = default;

    void setModelId(int modelId) { mModelId = modelId; }
    Q_INVOKABLE int getModelId() const { return mModelId; }

    virtual void setReverseFeed(bool reverse);
    bool isReverseFeed() const { return mReverseFeed; }

    virtual void setChronological(bool chronological);
    bool isChronological() const { return mChronological; }

    virtual QString getFeedName() const = 0;

    void setOverrideAdultVisibility(const QEnums::ContentVisibility visibility) { mOverrideAdultVisibility = visibility; }
    void clearOverrideAdultVisibility() { mOverrideAdultVisibility = {}; }

    Q_INVOKABLE void setOverrideLinkColor(const QString& color);
    Q_INVOKABLE void clearOverrideLinkColor();

    Q_INVOKABLE void reset();

    // NOTE: QModelIndex is a visible index, i.e. an index in the ListView in the UI
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    virtual bool isEndOfFeed() const { return mEndOfFeed; }
    virtual void setEndOfFeed(bool endOfFeed);

    const Post& getPost(int visibleIndex) const;
    Q_INVOKABLE void unfoldPosts(int startVisibleIndex);

    // Get the timestamp of the last post in the feed
    QDateTime lastTimestamp() const;

    // Returns the visible index of the last post >= timestamp, 0 if no such post exists
    // If  there are multiple posts with the same timestamp, then pick the one with
    // matching cid.
    Q_INVOKABLE int findTimestamp(QDateTime timestamp, const QString& cid) const;

    // Returns visible index of post, or -1 if post not found.
    int findPost(const QString& cid) const;

    int lastRowIndex() const;

    Q_INVOKABLE QDateTime getPostTimelineTimestamp(int visibleIndex) const;
    Q_INVOKABLE QString getPostCid(int visibleIndex) const;

    virtual void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

    virtual void setFeedError(const QString& error);
    void clearFeedError() { setFeedError({}); }
    const QString& getFeedError() const { return mFeedError; }

    bool isFilteredPostFeed() const { return mPostHideInfoMap; }
    Q_INVOKABLE ContentFilterStatsModel* createContentFilterStatsModel();

signals:
    void reverseFeedChanged();
    void endOfFeedChanged();
    void getFeedInProgressChanged();
    void feedErrorChanged();
    void chronologicalChanged();

protected:
    using TimelineFeed = std::deque<Post>;

    struct AbstractPage
    {
        TimelineFeed mFeed;
        bool mChronological = true;

        void pushPost(const Post& post);
        void chronoCheck();
        QDateTime firstTimestamp() const;
    };

    QHash<int, QByteArray> roleNames() const override;
    void beginRemoveRowsPhysical(int firstPhysicalIndex, int lastPhysicalIndex);
    void beginInsertRowsPhysical(int firstPhysicalIndex, int lastPhysicalIndex);

    int toPhysicalIndex(int visibleIndex) const;
    int toVisibleIndex(int physicalIndex, std::optional<int> feedSize = {}) const;

    void clearFeed();
    void deletePost(int visibleIndex);
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

    TimelineFeed mFeed;
    bool mReverseFeed = false;

    const QString& mUserDid;
    const IProfileStore& mMutedReposts;
    const IListStore& mFeedHide;
    const IContentFilter& mContentFilter;
    const IMatchWords& mMutedWords;
    const FocusHashtags& mFocusHashtags;
    HashtagIndex& mHashtags;
    ContentFilterStats mContentFilterStats{mFeedHide};
    const ContentFilterStats::PostHideInfoMap* mPostHideInfoMap = nullptr;
    int mModelId = -1;

private:
    static const QString NULL_STRING;
    static const ListStore NULL_LIST_STORE;
    static const ProfileStore NULL_PROFILE_STORE;
    static const ContentFilterShowAll NULL_CONTENT_FILTER;
    static const MutedWordsNoMutes NULL_MATCH_WORDS;
    static const FocusHashtags NULL_FOCUS_HASHTAGS;
    static HashtagIndex NULL_HASHTAG_INDEX;

    void indexHashtags(const Post& post);
    void identifyThreadPost(const Post& post);

    void postIsThreadChanged(const QString& postUri);
    void authorAdded(const QString& did);
    void labelerAdded(const QString& did);
    void listAdded(const QString& uri);

    BasicProfile getContentLabeler(QEnums::ContentVisibility visibility,
                                   const ContentLabelList& labels,
                                   int labelIndex) const;

    void flipPostsOrder();
    void reversePosts(int startPhysicalIndex, int endPhysicalIndex);

    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;
    std::optional<QEnums::ContentVisibility> mOverrideAdultVisibility;
    QString mOverrideLinkColor;
    bool mEndOfFeed = false;
    bool mGetFeedInProgress = false;
    QString mFeedError;
    bool mChronological = true;
};

}
