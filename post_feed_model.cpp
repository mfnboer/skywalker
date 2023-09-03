// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_feed_model.h"
#include <QQmlEngine>
#include <QQmlListProperty>

using namespace std::chrono_literals;

namespace Skywalker {

QCache<QString, CachedBasicProfile> PostFeedModel::sAuthorCache(1000);

void PostFeedModel::cacheAuthorProfile(const QString& did, const BasicProfile& profile)
{
    sAuthorCache.insert(did, new CachedBasicProfile(profile));
}

PostFeedModel::PostFeedModel(QObject* parent) :
    QAbstractListModel(parent)
{}

void PostFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (feed->mFeed.empty())
    {
        qDebug() << "Trying to set an empty feed";
        return;
    }

    if (mFeed.empty())
    {
        addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        return;
    }

    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
}

int PostFeedModel::prependFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (feed->mFeed.empty())
        return 0;

    if (mFeed.empty())
    {
        setFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        return 0;
    }

    return insertFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed), 0);
}

int PostFeedModel::gapFillFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int gapId)
{
    qDebug() << "Fill gap:" << gapId;

    if (!mGapIdIndexMap.count(gapId))
    {
        qWarning() << "Gap does not exist:" << gapId;
        return 0;
    }

    const int gapIndex = mGapIdIndexMap[gapId];
    mGapIdIndexMap.erase(gapId);

    if (gapIndex > mFeed.size())
    {
        qWarning() << "Gap:" << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return 0;
    }

    Q_ASSERT(mFeed[gapIndex].getGapId() == gapId);

    // Remove gap place holder
    beginRemoveRows({}, gapIndex, gapIndex);
    mFeed.erase(mFeed.begin() + gapIndex);
    addToIndices(-1, gapIndex);
    endRemoveRows();

    qDebug() << "Removed place holder post:" << gapIndex;
    logIndices();

    return insertFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed), gapIndex);
}

void PostFeedModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize)
{
    mFeed.insert(feedInsertIt, page.mFeed.begin(), page.mFeed.begin() + pageSize);

    for (const auto& post : page.mFeed)
    {
        const auto& cid = post.getCid();
        if (!cid.isEmpty())
        {
            mStoredCids.insert(cid);
            mStoredCidQueue.push(cid);
        }
    }

    cleanupStoredCids();
    qDebug() << "Stored cid set:" << mStoredCids.size() << "cid queue:" << mStoredCidQueue.size();
}

void PostFeedModel::cleanupStoredCids()
{
    while (mStoredCids.size() > MAX_TIMELINE_SIZE && !mStoredCidQueue.empty())
    {
        const auto& cid = mStoredCidQueue.front();
        mStoredCids.erase(cid);
        mStoredCidQueue.pop();
    }

    if (mStoredCidQueue.empty())
    {
        Q_ASSERT(mStoredCids.empty());
        qWarning() << "Stored cid set should have been empty:" << mStoredCids.size();
        mStoredCids.clear();
    }
}

int PostFeedModel::insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int insertIndex)
{
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    if (page->mFeed.empty())
    {
        qWarning() << "Page has no posts";
        return 0;
    }

    const auto overlapStart = findOverlapStart(*page, insertIndex);

    if (!overlapStart)
    {
        page->mFeed.push_back(Post::createGapPlaceHolder(page->mCursorNextPage));
        const int gapId = page->mFeed.back().getGapId();

        const size_t lastInsertIndex = insertIndex + page->mFeed.size() - 1;
        beginInsertRows({}, insertIndex, lastInsertIndex);
        insertPage(mFeed.begin() + insertIndex, *page, page->mFeed.size());
        addToIndices(page->mFeed.size(), insertIndex);
        mGapIdIndexMap[gapId] = lastInsertIndex;

        // The -1 offset on lastInsertIndex is for the place holder post.
        if (!page->mCursorNextPage.isEmpty())
            mIndexCursorMap[lastInsertIndex - 1] = page->mCursorNextPage;

        mIndexRawFeedMap[lastInsertIndex - 1] = std::move(page->mRawFeed);
        endInsertRows();

        qDebug() << "Full feed inserted, new size:" << mFeed.size();
        logIndices();
        return gapId;
    }

    if (*overlapStart == 0)
    {
        qDebug() << "Full overlap, no new posts";
        return 0;
    }

    const auto overlapEnd = findOverlapEnd(*page, insertIndex);

    beginInsertRows({}, insertIndex, insertIndex + *overlapStart - 1);
    insertPage(mFeed.begin() + insertIndex, *page, *overlapStart);
    addToIndices(*overlapStart, insertIndex);

    if (!page->mCursorNextPage.isEmpty() && overlapEnd)
        mIndexCursorMap[*overlapStart + *overlapEnd] = page->mCursorNextPage;

    // Remove unused overlap
    int firstUnusedRawIndex = -1;

    for (int i = 0; i < *overlapStart; ++i)
        firstUnusedRawIndex = std::max(firstUnusedRawIndex, page->mFeed[i].getRawIndex());

    Q_ASSERT(firstUnusedRawIndex >= 0);
    page->mRawFeed.erase(page->mRawFeed.begin() + firstUnusedRawIndex, page->mRawFeed.end());

    mIndexRawFeedMap[insertIndex + *overlapStart - 1] = std::move(page->mRawFeed);
    endInsertRows();

    qDebug() << "Inserted" << *overlapStart << "posts, new size:" << mFeed.size();
    logIndices();
    return 0;
}

void PostFeedModel::clear()
{
    beginRemoveRows({}, 0, mFeed.size() - 1);
    mFeed.clear();
    mIndexCursorMap.clear();
    mIndexRawFeedMap.clear();
    mGapIdIndexMap.clear();
    mStoredCids.clear();
    mStoredCidQueue = {};
    mEndOfFeed = false;
    endRemoveRows();
    qDebug() << "All posts removed";
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();

    if (feed->mFeed.empty())
        return;

    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    if (page->mFeed.empty())
    {
        qWarning() << "Page has no posts";
        return;
    }

    const size_t newRowCount = mFeed.size() + page->mFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    insertPage(mFeed.end(), *page, page->mFeed.size());

    if (!page->mCursorNextPage.isEmpty())
        mIndexCursorMap[mFeed.size() - 1] = page->mCursorNextPage;
    else
        mEndOfFeed = true;

    mIndexRawFeedMap[mFeed.size() - 1] = std::move(page->mRawFeed);
    endInsertRows();

    qDebug() << "New feed size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removeTailPosts(int size)
{
    if (size <= 0 || size >= mFeed.size())
        return;

    const auto removeIndexCursorIt = mIndexCursorMap.lower_bound(mFeed.size() - size - 1);

    if (removeIndexCursorIt == mIndexCursorMap.end())
    {
        qWarning() << "Cannot remove" << size << "posts";
        logIndices();
        return;
    }

    const size_t removeIndex = removeIndexCursorIt->first + 1;

    if (removeIndex >= mFeed.size())
    {
        qDebug() << "Cannot remove beyond end, removeIndex:" << removeIndex << "size:" << mFeed.size();
        logIndices();
        return;
    }

    const auto removeIndexRawFeedIt = mIndexRawFeedMap.lower_bound(removeIndex);

    beginRemoveRows({}, removeIndex, mFeed.size() - 1);
    removePosts(removeIndex, mFeed.size() - removeIndex);
    mIndexCursorMap.erase(std::next(removeIndexCursorIt), mIndexCursorMap.end());
    mIndexRawFeedMap.erase(removeIndexRawFeedIt, mIndexRawFeedMap.end());

    for (auto it = mGapIdIndexMap.begin(); it != mGapIdIndexMap.end(); )
    {
        if (it->second >= removeIndex)
            it = mGapIdIndexMap.erase(it);
        else
            ++it;
    }

    mEndOfFeed = false;
    endRemoveRows();

    qDebug() << "Removed tail rows, new size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removeHeadPosts(int size)
{
    if (size <= 0 || size >= mFeed.size())
        return;

    size_t removeEndIndex = size - 1;
    while (removeEndIndex < mFeed.size() - 1 && mFeed[removeEndIndex + 1].isPlaceHolder())
        ++removeEndIndex;

    if (removeEndIndex >= mFeed.size() - 1)
    {
        qWarning() << "Cannot remove beyond end, removeIndex:" << removeEndIndex << "size:" << mFeed.size();
        logIndices();
        return;
    }

    const auto removeEndIndexCursorIt = mIndexCursorMap.upper_bound(removeEndIndex);
    const auto removeEndIndexRawFeedIt = mIndexRawFeedMap.upper_bound(removeEndIndex);
    const size_t removeSize = removeEndIndex + 1;

    beginRemoveRows({}, 0, removeEndIndex);
    removePosts(0, removeSize);
    Q_ASSERT(!mFeed.front().isPlaceHolder());
    mIndexCursorMap.erase(mIndexCursorMap.begin(), removeEndIndexCursorIt);
    mIndexRawFeedMap.erase(mIndexRawFeedMap.begin(), removeEndIndexRawFeedIt);

    for (auto it = mGapIdIndexMap.begin(); it != mGapIdIndexMap.end(); )
    {
        if (it->second <= removeEndIndex)
            it = mGapIdIndexMap.erase(it);
        else
            ++it;
    }

    addToIndices(-removeSize, removeSize);
    endRemoveRows();

    qDebug() << "Removed head rows, new size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removePosts(int startIndex, int size)
{
    Q_ASSERT(startIndex >=0 && startIndex + size <= mFeed.size());

    // We leave the cid's in the chronological queues. They don't do
    // harm. At cleanup, the non-stored cid's will be removed from the queue.
    for (int i = startIndex; i < startIndex + size; ++i)
        mStoredCids.erase(mFeed[i].getCid());

    mFeed.erase(mFeed.begin() + startIndex, mFeed.begin() + startIndex + size);
}

QString PostFeedModel::getLastCursor() const
{
    if (mEndOfFeed || mIndexCursorMap.empty())
        return {};

    return mIndexCursorMap.rbegin()->second;
}

const Post* PostFeedModel::getGapPlaceHolder(int gapId) const
{
    const auto it = mGapIdIndexMap.find(gapId);

    if (it == mGapIdIndexMap.end())
    {
        qWarning() << "Gap does not exist:" << gapId;
        return nullptr;
    }

    const int gapIndex = it->second;

    if (gapIndex > mFeed.size())
    {
        qWarning() << "Gap:" << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return nullptr;
    }

    const auto* gap = &mFeed[gapIndex];
    Q_ASSERT(gap->getGapId() == gapId);
    return gap;
}

int PostFeedModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mFeed.size();
}

QVariant PostFeedModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mFeed.size())
        return {};

    const auto& post = mFeed[index.row()];

    switch (Role(role))
    {
    case Role::Author:
        return QVariant::fromValue(post.getAuthor());
    case Role::PostText:
        return post.getText();
    case Role::PostIndexedSecondsAgo:
    {
        const auto duration = QDateTime::currentDateTime() - post.getIndexedAt();
        return qint64(duration / 1000ms);
    }
    case Role::PostImages:
    {
        QList<ImageView> images;
        for (const auto& img : post.getImages())
            images.push_back(*img);

        return QVariant::fromValue(images);
    }
    case Role::PostExternal:
    {
        auto external = post.getExternalView();
        return external ? QVariant::fromValue(*external) : QVariant();
    }
    case Role::PostRepostedByName:
    {
        const auto& repostedBy = post.getRepostedBy();
        return repostedBy ? repostedBy->getName() : QVariant();
    }
    case Role::PostRecord:
    {
        auto record = post.getRecordView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::PostRecordWithMedia:
    {
        auto record = post.getRecordWithMediaView();
        return record ? QVariant::fromValue(*record) : QVariant();
    }
    case Role::PostType:
        return post.getPostType();
    case Role::PostGapId:
        return post.getGapId();
    case Role::PostIsReply:
        return post.isReply();
    case Role::PostParentInThread:
        return post.isParentInThread();
    case Role::PostReplyToAuthor:
    {
        const auto& author = post.getReplyToAuthor();
        return author ? QVariant::fromValue(*author) : QVariant();
    }
    case Role::PostReplyCount:
        return post.getReplyCount();
    case Role::PostRepostCount:
        return post.getRepostCount();
    case Role::PostLikeCount:
        return post.getLikeCount();
    case Role::EndOfFeed:
        return post.isEndOfFeed();
    default:
        qDebug() << "Uknown role requested:" << role;
        break;
    }

    return {};
}

QHash<int, QByteArray> PostFeedModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Author), "author" },
        { int(Role::PostText), "postText" },
        { int(Role::PostIndexedSecondsAgo), "postIndexedSecondsAgo" },
        { int(Role::PostRepostedByName), "postRepostedByName" },
        { int(Role::PostImages), "postImages" },
        { int(Role::PostExternal), "postExternal" },
        { int(Role::PostRecord), "postRecord" },
        { int(Role::PostRecordWithMedia), "postRecordWithMedia" },
        { int(Role::PostType), "postType" },
        { int(Role::PostGapId), "postGapId" },
        { int(Role::PostIsReply), "postIsReply" },
        { int(Role::PostParentInThread), "postParentInThread" },
        { int(Role::PostReplyToAuthor), "postReplyToAuthor" },
        { int(Role::PostReplyCount), "postReplyCount" },
        { int(Role::PostRepostCount), "postRepostCount" },
        { int(Role::PostLikeCount), "postLikeCount" },
        { int(Role::EndOfFeed), "endOfFeed" }
    };

    return roles;
}

void PostFeedModel::Page::addPost(const Post& post, bool isParent)
{
    mFeed.push_back(post);
    const auto& cid = post.getCid();

    if (!cid.isEmpty())
        mAddedCids.insert(post.getCid());

    if (isParent)
        mParentIndexMap[cid] = mFeed.size() - 1;
}

bool PostFeedModel::Page::tryAddToExistingThread(const Post& post, const PostReplyRef& replyRef)
{
    auto parentIt = mParentIndexMap.find(post.getCid());
    if (parentIt == mParentIndexMap.end())
        return false;

    // The post we add is already in the page, probably as a parent of another post

    const int parentIndex = parentIt->second;
    Q_ASSERT(parentIndex < mFeed.size());

    const auto oldPost = mFeed[parentIndex];

    // Replace the existing post by this one, as this post has a reply ref.
    // A parent post does not have the information
    mFeed[parentIndex] = post;
    mFeed[parentIndex].setReplyRefTimestamp(oldPost.getTimelineTimestamp());
    mFeed[parentIndex].setParentInThread(oldPost.isParentInThread());

    if (replyRef.mParent.getCid() == replyRef.mRoot.getCid())
    {
        // The root is already in this page
        mFeed[parentIndex].setPostType(oldPost.getPostType());
        mFeed[parentIndex].setParentInThread(true);
        return true;
    }

    mFeed[parentIndex].setPostType(QEnums::POST_REPLY);
    mFeed[parentIndex].setParentInThread(true);

    // Add parent of this parent
    mFeed.insert(mFeed.begin() + parentIndex, replyRef.mParent);
    mFeed[parentIndex].setReplyRefTimestamp(oldPost.getTimelineTimestamp());
    mFeed[parentIndex].setPostType(oldPost.getPostType());

    if (mFeed[parentIndex].getReplyToCid() == replyRef.mRoot.getCid())
        mFeed[parentIndex].setParentInThread(true);

    mAddedCids.insert(replyRef.mParent.getCid());
    mParentIndexMap.erase(parentIt);

    for (auto& [cid, idx] : mParentIndexMap)
    {
        if (idx > parentIndex)
            ++idx;
    }

    mParentIndexMap[replyRef.mParent.getCid()] = parentIndex;

    qDebug() << "Added parent (" << replyRef.mParent.getCid() << ") to existing thread:" << post.getCid();
    return true;
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    auto page = std::make_unique<Page>();
    page->mRawFeed = std::forward<ATProto::AppBskyFeed::PostFeed>(feed->mFeed);

    for (size_t i = 0; i < page->mRawFeed.size(); ++i)
    {
        const auto& feedEntry = page->mRawFeed[i];

        if (feedEntry->mPost->mRecordType == ATProto::RecordType::APP_BSKY_FEED_POST)
        {
            Post post(feedEntry.get(), i);

            // Due to reposting a post can show up multiple times in the feed.
            if (cidIsStored(post.getCid()))
                continue;

            const auto& author = *feedEntry->mPost->mAuthor;
            const BasicProfile authorProfile(author.mHandle, author.mDisplayName.value_or(""));
            cacheAuthorProfile(author.mDid, authorProfile);

            const auto& replyRef = post.getViewPostReplyRef();

            // Reposted replies are displayed without thread context
            if (replyRef && !post.isRepost())
            {
                bool rootAdded = false;
                const auto& rootCid = replyRef->mRoot.getCid();
                const auto& parentCid = replyRef->mParent.getCid();

                if (page->tryAddToExistingThread(post, *replyRef))
                    continue;

                if (rootCid != parentCid && !cidIsStored(rootCid) && !page->cidAdded(rootCid))
                {
                    page->addPost(replyRef->mRoot);
                    page->mFeed.back().setPostType(QEnums::POST_ROOT);
                    rootAdded = true;
                }

                // If the parent was seen already, but the root not, then show the parent
                // again for consistency of the thread.
                if ((!cidIsStored(parentCid) && !page->cidAdded(parentCid)) || rootAdded)
                {
                    page->addPost(replyRef->mParent, true);
                    auto& parentPost = page->mFeed.back();
                    parentPost.setPostType(rootAdded ? QEnums::POST_REPLY : QEnums::POST_ROOT);

                    // Determine author of the parent's parent
                    if (parentPost.getReplyToCid() == rootCid)
                    {
                        parentPost.setReplyToAuthor(replyRef->mRoot.getAuthor());
                        parentPost.setParentInThread(rootAdded);
                    }

                    post.setPostType(QEnums::POST_LAST_REPLY);
                    post.setParentInThread(true);
                }
            }
            else
            {
                // A post may have been added already as a parent/root of a reply
                if (page->cidAdded(post.getCid()))
                    continue;
            }

            page->addPost(post);
        }
        else
        {
            qWarning() << "Unsupported post record type:" << int(feedEntry->mPost->mRecordType);
        }
    }

    if (feed->mCursor && !feed->mCursor->isEmpty())
    {
        page->mCursorNextPage = *feed->mCursor;
    }
    else
    {
        if (!page->mFeed.empty())
            page->mFeed.back().setEndOfFeed(true);
    }

    qDebug() << "Author profile cache size:" << sAuthorCache.size();
    return page;
}

std::optional<size_t> PostFeedModel::findOverlapStart(const Page& page, size_t feedIndex) const
{
    Q_ASSERT(mFeed.size() > feedIndex);
    Q_ASSERT(!mFeed[feedIndex].isPlaceHolder());
    const auto& cidFirstStoredPost = mFeed[feedIndex].getCid();
    const auto& timestampFirstStoredPost = mFeed[feedIndex].getTimelineTimestamp();

    for (size_t i = 0; i < page.mFeed.size(); ++i)
    {
        const auto& post = page.mFeed[i];

        // Check on timestamp because of repost.
        // A repost will have the cid of the original post.
        // We also put the parent/root of a reply in the timeline with the
        // timestamp of the reply.
        if (cidFirstStoredPost == post.getCid() && timestampFirstStoredPost == post.getTimelineTimestamp())
        {
            qDebug() << "Matching overlap index found:" << i;
            return i;
        }

        if (timestampFirstStoredPost > post.getTimelineTimestamp())
        {
            qDebug() << "Overlap on timestamp found:" << i << timestampFirstStoredPost << post.getTimelineTimestamp();
            return i;
        }
    }

    // NOTE: the gap may be empty when the last post in the page is the predecessor of
    // the first post the stored feed. There is no way of knowing.
    qDebug() << "No overlap found, there is a gap";
    return {};
}

std::optional<size_t> PostFeedModel::findOverlapEnd(const Page& page, size_t feedIndex) const
{
    Q_ASSERT(!page.mFeed.empty());
    const auto& cidLastPagePost = page.mFeed.back().getCid();
    const auto& timestampLastPagePost = page.mFeed.back().getTimelineTimestamp();

    for (size_t i = feedIndex; i < mFeed.size(); ++ i)
    {
        const auto& post = mFeed[i];

        if (post.isPlaceHolder())
            continue;

        if (cidLastPagePost == post.getCid() && timestampLastPagePost == post.getTimelineTimestamp())
        {
            qDebug() << "Last matching overlap index found:" << i;
            return i;
        }

        if (timestampLastPagePost > post.getTimelineTimestamp())
        {
            qDebug() << "Overlap on timestamp found:" << i << timestampLastPagePost << post.getTimelineTimestamp();
            return i;
        }
    }

    qWarning() << "No overlap found, page exceeds end of stored feed";
    return {};
}

void PostFeedModel::addToIndices(size_t offset, size_t startAtIndex)
{
    std::map<size_t, QString> newCursorMap;
    for (const auto& [index, cursor] : mIndexCursorMap)
    {
        if (index >= startAtIndex)
            newCursorMap[index + offset] = cursor;
        else
            newCursorMap[index] = cursor;
    }

    mIndexCursorMap = std::move(newCursorMap);

    std::map<size_t, ATProto::AppBskyFeed::PostFeed> newRawFeedMap;
    for (auto& [index, rawFeed] : mIndexRawFeedMap)
    {
        if (index >= startAtIndex)
            newRawFeedMap[index + offset] = std::move(rawFeed);
        else
            newRawFeedMap[index] = std::move(rawFeed);
    }

    mIndexRawFeedMap = std::move(newRawFeedMap);

    for (auto& [gapId, index] : mGapIdIndexMap)
    {
        if (index >= startAtIndex)
            index += offset;
    }
}

void PostFeedModel::logIndices() const
{
    qDebug() << "INDEX CURSOR MAP:";
    for (const auto& [index, cursor] : mIndexCursorMap)
        qDebug() << "Index:" << index << "Cursor:" << cursor;

    qDebug() << "INDEX RAW FEED MAP:";
    for (const auto& [index, rawFeed] : mIndexRawFeedMap)
    {
        Q_ASSERT(!rawFeed.empty());
        qDebug() << "Index:" << index << "cid:" << rawFeed.front()->mPost->mCid
                 << "indexedAt:" << rawFeed.front()->mPost->mIndexedAt
                 << "size:" << rawFeed.size();
    }

    qDebug() << "GAP INDEX MAP:";
    for (const auto& [gapId, index] : mGapIdIndexMap)
        qDebug() << "Gap:" << gapId << "Index:" << index;
}

}
