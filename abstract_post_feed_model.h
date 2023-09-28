// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once

#include "local_post_model_changes.h"
#include "post.h"
#include "profile_store.h"
#include <QAbstractListModel>
#include <QCache>
#include <deque>
#include <queue>
#include <unordered_set>

namespace Skywalker {

class AbstractPostFeedModel : public QAbstractListModel, public LocalPostModelChanges
{
    Q_OBJECT
public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;
    static const QCache<QString, CachedBasicProfile>& getAuthorCache() { return sAuthorCache; }
    static void cacheAuthorProfile(const QString& did, const BasicProfile& profile);

    enum class Role {
        Author = Qt::UserRole + 1,
        PostUri,
        PostCid,
        PostText,
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
        PostLocallyDeleted,
        EndOfFeed
    };

    using Ptr = std::unique_ptr<AbstractPostFeedModel>;

    explicit AbstractPostFeedModel(const QString& userDid, const IProfileStore& following, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    const Post& getPost(int index) const { return mFeed.at(index); }
    void updatePostIndexTimestamps();

protected:
    static QCache<QString, CachedBasicProfile> sAuthorCache;

    QHash<int, QByteArray> roleNames() const override;
    void clearFeed();
    void storeCid(const QString& cid);
    void removeStoredCid(const QString& cid);
    void cleanupStoredCids();
    bool cidIsStored(const QString& cid) const { return mStoredCids.count(cid); }
    bool isEndOfFeed() const { return mEndOfFeed; }
    void setEndOfFeed(bool endOfFeed) { mEndOfFeed = endOfFeed; }

    virtual void postIndexTimestampChanged() override;
    virtual void likeCountChanged() override;
    virtual void likeUriChanged() override;
    virtual void replyCountChanged() override;
    virtual void repostCountChanged() override;
    virtual void repostUriChanged() override;

    using TimelineFeed = std::deque<Post>;
    TimelineFeed mFeed;

    const QString& mUserDid;
    const IProfileStore& mFollowing;

private:
    void changeData(const QList<int>& roles);

    // TODO: change to QCache
    // CID of posts stored in the timeline.
    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;

    bool mEndOfFeed = false;
};

}
