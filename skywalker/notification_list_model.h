// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "base_list_model.h"
#include "content_filter.h"
#include "follows_activity_store.h"
#include "local_post_model_changes.h"
#include "local_profile_changes.h"
#include "muted_words.h"
#include "notification.h"
#include "post_cache.h"
#include <atproto/lib/client.h>
#include <QAbstractListModel>
#include <deque>

namespace Skywalker {

class NotificationListModel : public QAbstractListModel,
                              public BaseListModel,
                              public LocalPostModelChanges,
                              public LocalProfileChanges
{
    Q_OBJECT
    Q_PROPERTY(bool getFeedInProgress READ isGetFeedInProgress NOTIFY getFeedInProgressChanged FINAL)
    Q_PROPERTY(bool priority READ getPriority NOTIFY priorityChanged FINAL)

public:
    using Ptr = std::unique_ptr<NotificationListModel>;
    using NotificationList = std::deque<Notification>;

    enum class Role {
        NotificationAuthor = Qt::UserRole + 1,
        NotificationOtherAuthors,
        NotificationAllAuthors,
        NotificationReason,
        NotificationReasonRaw,
        NotificationIsAggregatable,
        NotificationReasonSubjectUri,
        NotificationReasonSubjectCid,
        NotificationReasonPostText,
        NotificationReasonPostPlainText,
        NotificationReasonPostAuthor,
        NotificationReasonPostIsReply,
        NotificationReasonPostReplyToAuthor,
        NotificationReasonPostLanguages,
        NotificationReasonPostTimestamp,
        NotificationReasonPostHasUnknownEmbed,
        NotificationReasonPostUnknownEmbedType,
        NotificationReasonPostImages,
        NotificationReasonPostVideo,
        NotificationReasonPostExternal,
        NotificationReasonPostRecord,
        NotificationReasonPostRecordWithMedia,
        NotificationReasonPostNotFound,
        NotificationReasonPostLabels,
        NotificationReasonPostLocallyDeleted,
        NotificationTimestamp,
        NotificationSecondsAgo,
        NotificationIsRead,
        NotificationPostUri,
        NotificationCid,
        NotificationLabels,
        NotificationPostAuthor,
        NotificationPostText,
        NotificationPostPlainText,
        NotificationPostLanguages,
        NotificationPostTimestamp,
        NotificationPostHasUnknownEmbed,
        NotificationPostUnknownEmbedType,
        NotificationPostImages,
        NotificationPostVideo,
        NotificationPostExternal,
        NotificationPostRecord,
        NotificationPostRecordWithMedia,
        NotificationPostReplyRootAuthorDid,
        NotificationPostReplyRootUri,
        NotificationPostReplyRootCid,
        NotificationPostMentionDids,
        NotificationPostRepostUri,
        NotificationPostLikeUri,
        NotificationPostLikeTransient,
        NotificationPostThreadMuted,
        NotificationPostReplyDisabled,
        NotificationPostEmbeddingDisabled,
        NotificationPostViewerStatePinned,
        NotificationPostThreadgateUri,
        NotificationPostReplyRestriction,
        NotificationPostReplyRestrictionLists,
        NotificationPostHiddenReplies,
        NotificationPostIsHiddenReply,
        NotificationPostRepostCount,
        NotificationPostLikeCount,
        NotificationPostQuoteCount,
        NotificationPostReplyCount,
        NotificationPostBookmarked,
        NotificationPostBookmarkTransient,
        NotificationPostNotFound,
        NotificationPostBlocked,
        NotificationPostLabels,
        NotificationPostContentVisibility,
        NotificationPostContentWarning,
        NotificationPostContentLabeler,
        NotificationPostMutedReason,
        NotificationPostIsReply,
        ReplyToAuthor,
        NotificationInviteCode,
        NotificationInviteCodeUsedBy,
        EndOfList
    };

    explicit NotificationListModel(const ContentFilter& contentFilter,
                                   const MutedWords& mutedWords, FollowsActivityStore* followsActivityStore,
                                   QObject* parent = nullptr);

    void setModelId(int modelId) { mModelId = modelId; }
    int getModelId() const { return mModelId; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE void clear();
    bool addNotifications(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr notifications,
                          ATProto::Client::SharedPtr bsky, bool clearFirst = false,
                          const std::function<void()>& doneCb = nullptr);
    QString addNotifications(ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr convoListOutput,
                          const QString& lastRev, const QString& userDid);
    const QString& getCursor() const { return mCursor; }
    bool getPriority() const { return mPriority; }
    void setPriority(bool priority);
    bool isEndOfList() const { return mCursor.isEmpty(); }

    Q_INVOKABLE bool notificationsLoaded() const { return !mList.empty(); }

    // Q_INVOKABLE void addInviteCodeUsageNofications(InviteCodeStore* inviteCodeStore);
    Q_INVOKABLE void dismissInviteCodeUsageNotification(int index);
    int getInviteCodeUsageNotificationCount() const { return (int)mInviteCodeUsedNotifications.size(); }

    Q_INVOKABLE void dismissNewLabelNotification(int index);
    int addNewLabelsNotifications(const std::unordered_map<QString, BasicProfile>& labelerProfiles);

    void setNotificationsSeen(bool seen);
    Q_INVOKABLE void updateRead();

    int getIndexOldestUnread() const;
    const NotificationList& getNotifications() const { return mList; }
    QDateTime getTimestampLatestNotifcation() const;
    const PostCache& getPostCache() const { return mPostCache; }
    const PostCache& getReasonPostCache() const { return mReasonPostCache; }
    void enableRetrieveNotificationPosts(bool enable) { mRetrieveNotificationPosts = enable; }

    void setGetFeedInProgress(bool inProgress);
    bool isGetFeedInProgress() const { return mGetFeedInProgress; }

signals:
    void priorityChanged();
    void getFeedInProgressChanged();

protected:
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
    virtual void feedbackChanged() override {};
    virtual void feedbackTransientChanged() override {};
    virtual void detachedRecordChanged() override;
    virtual void reAttachedRecordChanged() override;
    virtual void viewerStatePinnedChanged() override;
    virtual void postDeletedChanged() override;

    // LocalProfileChanges
    virtual void profileChanged() override {};
    virtual void locallyBlockedChanged() override;

    QHash<int, QByteArray> roleNames() const override;

private:
    void reportActivity(const Notification& notification) const;
    NotificationList createNotificationList(const ATProto::AppBskyNotification::Notification::List& rawList) const;
    void filterNotificationList(NotificationList& list) const;
    void addNotificationList(const NotificationList& list, bool clearFirst);
    void addConvoLastMessage(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& lastRev, const QString& userDid);
    void addConvoLastReaction(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& lastRev, const QString& userDid);

    // Get the posts for LIKE, FOLLOW and REPOST notifications
    void getPosts(ATProto::Client& bsky, const NotificationList& list, const std::function<void()>& cb);
    void getPosts(ATProto::Client& bsky, std::unordered_set<QString> uris, const std::function<void()>& cb);

    void changeData(const QList<int>& roles) override;
    void clearLocalState();
    void clearRows();
    void addInviteCodeUsageNotificationRows();
    void addNewLabelsNotificationRows();
    void updateNewLabelsNotifications();
    void updateInviteCodeUser(const BasicProfile& profile);

    BasicProfile getContentLabeler(QEnums::ContentVisibility visibility,
                                   const ContentLabelList& labels,
                                   int labelIndex) const;

    const ContentFilter& mContentFilter;
    const MutedWords& mMutedWords;
    FollowsActivityStore* mFollowsActivityStore;

    NotificationList mList;
    QString mCursor;
    bool mPriority = false;

    // This cache must be emptied when the notifications are refreshed, because
    // the counts (like, reposts, replies) will change over time and are displayed.
    PostCache mPostCache;
    bool mRetrieveNotificationPosts = true;

    // Posts in this cache can be kept for a long time
    PostCache mReasonPostCache;

    NotificationList mInviteCodeUsedNotifications;
    NotificationList mNewLabelsNotifications;
    bool mNotificationsSeen = false;
    bool mGetFeedInProgress = false;
    int mModelId = -1;
};

}
