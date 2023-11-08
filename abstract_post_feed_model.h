// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "bookmarks.h"
#include "content_filter.h"
#include "local_post_model_changes.h"
#include "post.h"
#include "profile_store.h"
#include <QAbstractListModel>
#include <deque>
#include <queue>
#include <unordered_set>

namespace Skywalker {

class AbstractPostFeedModel : public QAbstractListModel, public LocalPostModelChanges
{
    Q_OBJECT
public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;

    enum class Role {
        Author = Qt::UserRole + 1,
        PostUri,
        PostCid,
        PostText, // Formatted
        PostPlainText,
        PostIndexedDateTime,
        PostRepostedByName,
        PostImages,
        PostExternal,
        PostRecord,
        PostRecordWithMedia,
        PostType,
        PostThreadType,
        PostIsPlaceHolder,
        PostGapId,
        PostNotFound,
        PostBlocked,
        PostNotSupported,
        PostUnsupportedType,
        PostIsReply,
        PostParentInThread,
        PostReplyToAuthor,
        PostReplyRootUri,
        PostReplyRootCid,
        PostReplyCount,
        PostRepostCount,
        PostLikeCount,
        PostRepostUri,
        PostLikeUri,
        PostBookmarked,
        PostLabels,
        PostContentVisibility,
        PostContentWarning,
        PostLocallyDeleted,
        EndOfFeed
    };

    using Ptr = std::unique_ptr<AbstractPostFeedModel>;

    AbstractPostFeedModel(const QString& userDid, const IProfileStore& following,
                          const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                          QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    const Post& getPost(int index) const { return mFeed.at(index); }
    void updatePostIndexTimestamps();

protected:
    QHash<int, QByteArray> roleNames() const override;
    void clearFeed();
    void storeCid(const QString& cid);
    void removeStoredCid(const QString& cid);
    void cleanupStoredCids();
    bool cidIsStored(const QString& cid) const { return mStoredCids.count(cid); }
    bool isEndOfFeed() const { return mEndOfFeed; }
    void setEndOfFeed(bool endOfFeed) { mEndOfFeed = endOfFeed; }
    bool mustHideContent(const Post& post) const;

    virtual void postIndexTimestampChanged() override;
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void replyCountChanged() override;
    virtual void repostCountChanged() override;
    virtual void repostUriChanged() override;
    virtual void postDeletedChanged() override;

    using TimelineFeed = std::deque<Post>;
    TimelineFeed mFeed;

    const QString& mUserDid;
    const IProfileStore& mFollowing;
    const ContentFilter& mContentFilter;
    const Bookmarks& mBookmarks;

private:
    void postBookmarkedChanged();
    void changeData(const QList<int>& roles);

    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;

    bool mEndOfFeed = false;
};

}
