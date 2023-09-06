// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once

#include "post.h"
#include <QAbstractListModel>
#include <QCache>
#include <deque>
#include <queue>
#include <unordered_set>

namespace Skywalker {

class AbstractPostFeedModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static constexpr int MAX_TIMELINE_SIZE = 5000;
    static const QCache<QString, CachedBasicProfile>& getAuthorCache() { return sAuthorCache; }

    enum class Role {
        Author = Qt::UserRole + 1,
        PostUri,
        PostText,
        PostIndexedSecondsAgo,
        PostRepostedByName,
        PostImages,
        PostExternal,
        PostRecord,
        PostRecordWithMedia,
        PostType,
        PostGapId,
        PostIsReply,
        PostParentInThread,
        PostReplyToAuthor,
        PostReplyCount,
        PostRepostCount,
        PostLikeCount,
        EndOfFeed
    };

    using Ptr = std::unique_ptr<AbstractPostFeedModel>;

    explicit AbstractPostFeedModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    static void cacheAuthorProfile(const QString& did, const BasicProfile& profile);
    static QCache<QString, CachedBasicProfile> sAuthorCache;

    QHash<int, QByteArray> roleNames() const override;
    void clearFeed();
    void storeCid(const QString& cid);
    void removeStoredCid(const QString& cid);
    void cleanupStoredCids();
    bool cidIsStored(const QString& cid) const { return mStoredCids.count(cid); }
    bool isEndOfFeed() const { return mEndOfFeed; }
    void setEndOfFeed(bool endOfFeed) { mEndOfFeed = endOfFeed; }

    using TimelineFeed = std::deque<Post>;
    TimelineFeed mFeed;

private:
    // TODO: change to QCache
    // CID of posts stored in the timeline.
    std::unordered_set<QString> mStoredCids;
    std::queue<QString> mStoredCidQueue;

    bool mEndOfFeed = false;
};

}
