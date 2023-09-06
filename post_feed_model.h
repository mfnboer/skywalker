// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace Skywalker {

class PostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<PostFeedModel>;

    explicit PostFeedModel(QObject* parent = nullptr);

    void setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
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

private:
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

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    Page::Ptr createPage(ATProto::AppBskyFeed::ThreadViewPost::Ptr&& thread);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    // Returns gap id if insertion created a gap in the feed.
    int insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int insertIndex);

    // Returns an index in the page feed
    std::optional<size_t> findOverlapStart(const Page& page, size_t feedIndex) const;

    // Return an index in mFeed
    std::optional<size_t> findOverlapEnd(const Page& page, size_t feedIndex) const;

    void addToIndices(size_t offset, size_t startAtIndex);
    void logIndices() const;

    // The index is the last (non-filtered) post from a received page. The cursor is to get
    // the next page.
    std::map<size_t, QString> mIndexCursorMap; // cursor to post at next index

    // The index of the last post that depends on the raw PostFeed. All posts from the previous
    // index in this map till this one depend on it. The raw PostFeed must be kept alive as
    // long it has dependend posts.
    std::map<size_t, ATProto::AppBskyFeed::PostFeed> mIndexRawFeedMap;

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;
};

}
