// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include <atproto/lib/user_preferences.h>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace Skywalker {

class PostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<PostFeedModel>;

    explicit PostFeedModel(const QString& userDid, const IProfileStore& following,
                           const ContentFilter& contentFilter,
                           const ATProto::UserPreferences& userPrefs,
                           QObject* parent = nullptr);

    // Return the new index of the current top post.
    // If the feed was empty then -1 is returned.
    int setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);

    void addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);

    // Returns gap id if prepending created a gap in the feed.
    // Returns 0 otherwise.
    int prependFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);

    // Returns new gap id if the gap was not fully filled, i.e. there is a new gap.
    // Returns 0 otherwise.
    int gapFillFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int gapId);

    void removeTailPosts(int size);
    void removeHeadPosts(int size);
    void removePosts(int startIndex, int size);

    QString getLastCursor() const;
    const Post* getGapPlaceHolder(int gapId) const;

    // Get the timestamp of the last post in the feed
    QDateTime lastTimestamp() const;

    // Returns the index of the first post <= timestamp
    int findTimestamp(QDateTime timestamp) const;

    void clear();

private:
    struct CidTimestamp
    {
        QString mCid;
        QDateTime mTimestamp;
        QString mRepostedByDid;
        QEnums::PostType mPostType;
    };

    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        ATProto::AppBskyFeed::PostFeed mRawFeed;
        QString mCursorNextPage;
        std::unordered_set<QString> mAddedCids;
        std::unordered_map<QString, int> mParentIndexMap;

        void addPost(const Post& post, bool isParent = false);
        bool cidAdded(const QString& cid) const { return mAddedCids.count(cid); }
        bool tryAddToExistingThread(const Post& post, const PostReplyRef& replyRef);
    };

    bool mustShowReply(const Post& post, const std::optional<PostReplyRef>& replyRef) const;
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    // Returns gap id if insertion created a gap in the feed.
    int insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int insertIndex);

    // Returns an index in the page feed
    std::optional<size_t> findOverlapStart(const Page& page, size_t feedIndex) const;

    // Return an index in mFeed
    std::optional<size_t> findOverlapEnd(const Page& page, size_t feedIndex) const;

    void addToIndices(size_t offset, size_t startAtIndex);
    void logIndices() const;
    void setTopNCids();
    int topNPostIndex(const Post& post, bool checkTimestamp) const;

    const ATProto::UserPreferences& mUserPreferences;

    // The index is the last (non-filtered) post from a received page. The cursor is to get
    // the next page.
    std::map<size_t, QString> mIndexCursorMap; // cursor to post at next index

    // The index of the last post that depends on the raw PostFeed. All posts from the previous
    // index in this map till this one depend on it. The raw PostFeed must be kept alive as
    // long it has dependend posts.
    std::map<size_t, ATProto::AppBskyFeed::PostFeed> mIndexRawFeedMap;

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;

    // The top N cids from the posts in the feed before last clear.
    std::vector<CidTimestamp> mTopNCids;

    // Number of posts that have been prepended to the feed since the last clear.
    size_t mPrependPostCount = 0;
};

}
