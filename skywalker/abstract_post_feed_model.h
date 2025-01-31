// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "bookmarks.h"
#include "content_filter.h"
#include "hashtag_index.h"
#include "local_post_model_changes.h"
#include "local_profile_changes.h"
#include "post.h"
#include "profile_store.h"
#include <QAbstractListModel>
#include <deque>
#include <queue>
#include <unordered_set>

namespace Skywalker {

class FocusHashtags;

class AbstractPostFeedModel : public QAbstractListModel,
                              public LocalPostModelChanges,
                              public LocalProfileChanges
{
    Q_OBJECT
    Q_PROPERTY(bool endOfFeed READ isEndOfFeed NOTIFY endOfFeedChanged FINAL)
    QML_ELEMENT

public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;

    enum class Role {
        Author = Qt::UserRole + 1,
        PostUri,
        PostCid,
        PostText, // Formatted
        PostPlainText,
        PostLanguages,
        PostIndexedDateTime,
        PostIndexedSecondsAgo,
        PostRepostedByAuthor,
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
        PostBookmarkNotFound,
        PostLabels,
        PostContentVisibility,
        PostContentWarning,
        PostMutedReason,
        PostHighlightColor,
        PostIsPinned,
        PostLocallyDeleted,
        EndOfFeed
    };
    Q_ENUM(Role)

    using Ptr = std::unique_ptr<AbstractPostFeedModel>;

    AbstractPostFeedModel(const QString& userDid, const IProfileStore& following,
                          const IProfileStore& mutedReposts,
                          const IProfileStore& feedHide,
                          const IContentFilter& contentFilter, const Bookmarks& bookmarks,
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

    const Post& getPost(int index) const { return mFeed.at(index); }
    void preprocess(const Post& post);

    Q_INVOKABLE void unfoldPosts(int startIndex);

signals:
    void endOfFeedChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;
    void clearFeed();
    void deletePost(int index);
    void storeCid(const QString& cid);
    void removeStoredCid(const QString& cid);
    void cleanupStoredCids();
    bool cidIsStored(const QString& cid) const { return mStoredCids.count(cid); }

    virtual bool mustHideContent(const Post& post) const;

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
    virtual void detachedRecordChanged() override;
    virtual void reAttachedRecordChanged() override;
    virtual void viewerStatePinnedChanged() override;
    virtual void postDeletedChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override;
    virtual void locallyBlockedChanged() override;

    void changeData(const QList<int>& roles);

    using TimelineFeed = std::deque<Post>;
    TimelineFeed mFeed;

    const QString& mUserDid;
    const IProfileStore& mFollowing;
    const IProfileStore& mMutedReposts;
    const IProfileStore& mFeedHide;
    const IContentFilter& mContentFilter;
    const Bookmarks& mBookmarks;
    const IMatchWords& mMutedWords;
    const FocusHashtags& mFocusHashtags;
    HashtagIndex& mHashtags;
    int mModelId = -1;

private:
    void postBookmarkedChanged();

    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;
    std::optional<QEnums::ContentVisibility> mOverrideAdultVisibility;
    QString mOverrideLinkColor;
    bool mEndOfFeed = false;
};

}
