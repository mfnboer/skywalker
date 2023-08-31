// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <QAbstractListModel>
#include <deque>
#include <map>
#include <unordered_map>

namespace Skywalker {

class PostFeedModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Author = Qt::UserRole + 1,
        PostText,
        PostIndexedSecondsAgo,
        PostRepostedByName,
        PostImages,
        PostExternal,
        PostRecord,
        PostRecordWithMedia,
        PostType,
        PostGapId,
        EndOfFeed
    };

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

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QString getLastCursor() const;
    const Post* getGapPlaceHolder(int gapId) const;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        ATProto::AppBskyFeed::PostFeed mRawFeed;
        QString mCursorNextPage;
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed) const;

    // Returns gap id if insertion created a gap in the feed.
    int insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int insertIndex);

    // Returns an index in the page feed
    std::optional<size_t> findOverlapStart(const Page& page, size_t feedIndex) const;

    // Return an index in mFeed
    std::optional<size_t> findOverlapEnd(const Page& page, size_t feedIndex) const;

    void addToIndices(size_t offset, size_t startAtIndex);
    void logIndices() const;

    std::deque<Post> mFeed;

    // The index is the last (non-filtered) post from a received page. The cursor is to get
    // the next page.
    std::map<size_t, QString> mIndexCursorMap; // cursor to post at next index

    // The index of the last post that depends on the raw PostFeed. All posts from the previous
    // index in this map till this one depend on it. The raw PostFeed must be kept alive as
    // long it has dependend posts.
    std::map<size_t, ATProto::AppBskyFeed::PostFeed> mIndexRawFeedMap;

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;

    bool mEndOfFeed = false;
};

}
