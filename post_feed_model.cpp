// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_feed_model.h"
#include "author_cache.h"
#include <algorithm>

namespace Skywalker {

static constexpr char const* HOME_FEED = "home";

PostFeedModel::PostFeedModel(const QString& userDid, const IProfileStore& following,
                             const ContentFilter& contentFilter,
                             const ATProto::UserPreferences& userPrefs,
                             QObject* parent) :
    AbstractPostFeedModel(userDid, following, contentFilter, parent),
    mUserPreferences(userPrefs)
{}

int PostFeedModel::setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (mFeed.empty())
    {
        addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        return -1;
    }

    const QString topCid = mFeed.front().getCid();
    setTopNCids();
    clear();
    addFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    int prevTopIndex = -1;

    if (!topCid.isEmpty())
    {
        for (int i = 0; i < (int)mFeed.size(); ++i)
        {
            if (topCid == mFeed[i].getCid())
            {
                prevTopIndex = i;
                break;
            }
        }
    }

    mTopNCids.clear();
    return prevTopIndex;
}

int PostFeedModel::prependFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    if (feed->mFeed.empty())
        return 0;

    if (mFeed.empty())
    {
        setFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));
        mPrependPostCount = mFeed.size();
        qDebug() << "Prepened post count:" << mPrependPostCount;
        return 0;
    }

    const size_t prevSize = mFeed.size();
    const int gapId = insertFeed(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed), 0);
    mPrependPostCount += mFeed.size() - prevSize;
    qDebug() << "Prepened post count:" << mPrependPostCount;

    return gapId;
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

    if (gapIndex > (int)mFeed.size())
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
            storeCid(cid);
    }

    cleanupStoredCids();
}

int PostFeedModel::insertFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed, int insertIndex)
{
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    if (page->mFeed.empty())
    {
        qDebug() << "Page has no posts";
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

    for (size_t i = 0; i < *overlapStart; ++i)
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
    clearFeed();
    mIndexCursorMap.clear();
    mIndexRawFeedMap.clear();
    mGapIdIndexMap.clear();
    endRemoveRows();

    mPrependPostCount = 0;
    qDebug() << "All posts removed";
}

void PostFeedModel::addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    qDebug() << "Add raw posts:" << feed->mFeed.size();
    auto page = createPage(std::forward<ATProto::AppBskyFeed::OutputFeed::Ptr>(feed));

    if (!page->mFeed.empty())
    {
        const size_t newRowCount = mFeed.size() + page->mFeed.size();

        beginInsertRows({}, mFeed.size(), newRowCount - 1);
        insertPage(mFeed.end(), *page, page->mFeed.size());
        mIndexRawFeedMap[mFeed.size() - 1] = std::move(page->mRawFeed);
        endInsertRows();

        qDebug() << "New feed size:" << mFeed.size();
    }
    else
    {
        qDebug() << "All posts have been filtered from page";
    }

    if (!page->mCursorNextPage.isEmpty())
    {
        mIndexCursorMap[mFeed.size() - 1] = page->mCursorNextPage;
    }
    else
    {
        setEndOfFeed(true);

        if (page->mFeed.empty() && !mFeed.empty())
        {
            // Set end of feed indication on existing row.
            mFeed.back().setEndOfFeed(true);
            const auto index = createIndex(mFeed.size() - 1, 0);
            emit dataChanged(index, index, { int(Role::EndOfFeed) });
        }
    }

    logIndices();
}

void PostFeedModel::removeTailPosts(int size)
{
    if (size <= 0 || size >= (int)mFeed.size())
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

    setEndOfFeed(false);
    endRemoveRows();

    qDebug() << "Removed tail rows, new size:" << mFeed.size();
    logIndices();
}

void PostFeedModel::removeHeadPosts(int size)
{
    if (size <= 0 || size >= (int)mFeed.size())
        return;

    size_t removeEndIndex = size - 1;
    while (removeEndIndex < mFeed.size() - 1 && mFeed[removeEndIndex + 1].isGap())
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
    Q_ASSERT(!mFeed.front().isGap());
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
    Q_ASSERT(startIndex >=0 && startIndex + size <= (int)mFeed.size());

    for (int i = startIndex; i < startIndex + size; ++i)
        removeStoredCid(mFeed[i].getCid());

    mFeed.erase(mFeed.begin() + startIndex, mFeed.begin() + startIndex + size);
}

QString PostFeedModel::getLastCursor() const
{
    if (isEndOfFeed() || mIndexCursorMap.empty())
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

    if (gapIndex > (int)mFeed.size())
    {
        qWarning() << "Gap:" << gapId << "index:" << gapIndex << "beyond feed size" << mFeed.size();
        return nullptr;
    }

    const auto* gap = &mFeed[gapIndex];
    Q_ASSERT(gap->getGapId() == gapId);
    return gap;
}

QDateTime PostFeedModel::lastTimestamp() const
{
    return !mFeed.empty() ? mFeed.back().getTimelineTimestamp() : QDateTime();
}

int PostFeedModel::findTimestamp(QDateTime timestamp) const
{
    const Post dummy;
    auto it = std::lower_bound(mFeed.begin(), mFeed.end(), dummy,
            [timestamp](const Post& post, const Post&){
                if (post.isPlaceHolder())
                    return true; // place holder has no timestamp

                return post.getTimelineTimestamp() > timestamp;
            });

    return it != mFeed.end() ? it - mFeed.begin() : -1;
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
    Q_ASSERT(parentIndex < (int)mFeed.size());

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

    //qDebug() << "Added parent (" << replyRef.mParent.getCid() << ") to existing thread:" << post.getCid();
    return true;
}

bool PostFeedModel::mustShowReply(const Post& post, const std::optional<PostReplyRef>& replyRef) const
{
    const auto& feedViewPref = mUserPreferences.getFeedViewPref(HOME_FEED);

    if (feedViewPref.mHideReplies)
        return false;

    // Always show the replies of the user.
    if (post.getAuthor().getDid() == mUserDid)
        return true;

    if (feedViewPref.mHideRepliesByUnfollowed)
    {
        // In case of blocked posts there is no reply ref.
        // Surely someone that blocks you is not a friend of yours.
        if (!replyRef)
            return false;

        // Do not show replies to blocked and not-found posts
        if (replyRef->mParent.isPlaceHolder())
            return false;

        const auto parentDid = replyRef->mParent.getAuthor().getDid();

        // Always show replies to the user
        if (parentDid == mUserDid)
            return true;

        const auto rootDid = replyRef->mRoot.getAuthor().getDid();

        // Always show replies in a thread from the user
        if (rootDid == mUserDid)
            return true;

        if (!mFollowing.contains(parentDid))
            return false;
    }

    return true;
}

PostFeedModel::Page::Ptr PostFeedModel::createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed)
{
    const auto& feedViewPref = mUserPreferences.getFeedViewPref(HOME_FEED);
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

            if (feedViewPref.mHideReposts && post.isRepost())
                continue;

            if (feedViewPref.mHideQuotePosts && post.isQuotePost())
                continue;

            if (mustHideContent(post))
                continue;

            const BasicProfile author(feedEntry->mPost->mAuthor.get());
            AuthorCache::instance().put(author);

            const auto& replyRef = post.getViewPostReplyRef();

            // Reposted replies are displayed without thread context
            if (replyRef && !post.isRepost())
            {
                // If a reply fits in an existing thread then always show it as it provides
                // context to the user. The leaf of this thread is a reply that passed
                // through the filter settings.
                if (page->tryAddToExistingThread(post, *replyRef))
                    continue;

                if (!mustShowReply(post, replyRef))
                    continue;

                bool rootAdded = false;
                const auto& rootCid = replyRef->mRoot.getCid();
                const auto& parentCid = replyRef->mParent.getCid();

                const int postTopNIndex = topNPostIndex(post, true);

                // Do not check the timestamp of root and parent as these are set
                // to the timestamp of the post itself and not the timestamp that they
                // may have had before.
                const int rootTopNIndex = topNPostIndex(replyRef->mRoot, false);
                const int parentTopNIndex = topNPostIndex(replyRef->mParent, false);
                const bool allOutSideTopN = postTopNIndex == -1 && rootTopNIndex == -1 && parentTopNIndex == -1;
                const bool allInTopN = postTopNIndex >=0 && rootTopNIndex >= 0 && parentTopNIndex >= 0;

                if (!rootCid.isEmpty() && rootCid != parentCid && !cidIsStored(rootCid) && !page->cidAdded(rootCid))
                {
                    // Do not allow reordering (replies/parent/root) for the top-N posts.
                    // These were visible to the user before refreshing the timeline.
                    // Changing the order of these posts is annoying to the user.

                    if (allOutSideTopN || (allInTopN && rootTopNIndex < postTopNIndex))
                    {
                        page->addPost(replyRef->mRoot);
                        page->mFeed.back().setPostType(QEnums::POST_ROOT);
                        rootAdded = true;
                    }
                }

                // If the parent was seen already, but the root not, then show the parent
                // again for consistency of the thread.
                if ((!parentCid.isEmpty() && !cidIsStored(parentCid) && !page->cidAdded(parentCid)) || rootAdded)
                {
                    if (allOutSideTopN || (allInTopN && parentTopNIndex < postTopNIndex))
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
            }
            else if (post.isReply() && !post.isRepost())
            {
                // A post can still be a reply even if there is no reply reference.
                // The reference may be missing due to blocked posts.
                if (!mustShowReply(post, {}))
                    continue;
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
            page->addPost(Post::createNotSupported(feedEntry->mPost->mRawRecordType));
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

    return page;
}

std::optional<size_t> PostFeedModel::findOverlapStart(const Page& page, size_t feedIndex) const
{
    Q_ASSERT(mFeed.size() > feedIndex);
    QString cidFirstStoredPost;
    QDateTime timestampFirstStoredPost;

    for (const auto& post : mFeed)
    {
        if (post.isPlaceHolder())
            continue;

        cidFirstStoredPost = post.getCid();
        timestampFirstStoredPost = post.getTimelineTimestamp();
        break;
    }

    if (cidFirstStoredPost.isEmpty())
    {
        qWarning() << "There are not real posts in the feed!";
        return {};
    }

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

void PostFeedModel::setTopNCids()
{
    mTopNCids.clear();

    for (int i = 0; i < std::min({10, (int)mFeed.size(), (int)mPrependPostCount}); ++i)
    {
        const auto& post = mFeed[i];
        const QString& cid = post.getCid();

        if (!cid.isEmpty())
        {
            const auto repostedBy = post.getRepostedBy();
            const QString didRepostedBy = repostedBy ? repostedBy->getDid() : QString();
            mTopNCids.push_back(CidTimestamp{
                                             cid,
                                             post.getTimelineTimestamp(),
                                             didRepostedBy,
                                             post.getPostType() });
        }
    }
}

int PostFeedModel::topNPostIndex(const Post& post, bool checkTimestamp) const
{
    if (mTopNCids.empty())
        return -1;

    if (checkTimestamp)
    {
        const auto timestamp = post.getTimelineTimestamp();

        if (timestamp > mTopNCids.front().mTimestamp)
            return -1;

        if (timestamp < mTopNCids.back().mTimestamp)
            return -1;
    }

    for (int i = 0; i < (int)mTopNCids.size(); ++i)
    {
        const auto& topNPost = mTopNCids[i];
        const auto repostedBy = post.getRepostedBy();
        const QString didRepostedBy = repostedBy ? repostedBy->getDid() : QString();

        if (topNPost.mCid == post.getCid() && topNPost.mRepostedByDid == didRepostedBy)
            return i;
    }

    if (checkTimestamp)
        qDebug() << "Post is in top-N time interval, but not present:" << post.getCid() << post.isReply() << post.getText();

    return -1;
}

}
