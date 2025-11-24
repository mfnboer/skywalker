// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_thread_model.h"
#include "author_cache.h"
#include "list_store.h"
#include "thread_unroller.h"

namespace Skywalker {

PostThreadModel::PostThreadModel(const QString& threadEntryUri, QEnums::PostThreadType postThreadType,
                                 QEnums::ReplyOrder replyOrder,
                                 const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const ContentFilter& contentFilter,
                                 const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, ListStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mThreadEntryUri(threadEntryUri),
    mUnrollThread(postThreadType == QEnums::POST_THREAD_UNROLLED),
    mOnlyEntryAuthorPosts(postThreadType == QEnums::POST_THREAD_UNROLLED || postThreadType == QEnums::POST_THREAD_ENTRY_AUTHOR_POSTS),
    mPostThreadType(postThreadType),
    mReplyOrder(replyOrder)
{}

void PostThreadModel::insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize)
{
    mFeed.insert(feedInsertIt, page.mFeed.begin(), page.mFeed.begin() + pageSize);
}

int PostThreadModel::setPostThread(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread)
{
    if (!mFeed.empty())
        clear();

    setThreadgateView(thread->mThreadgate);
    auto page = createPage(thread, false);

    if (page->mFeed.empty())
    {
        qWarning() << "Page has no posts";
        return -1;
    }

    const size_t newRowCount = page->mFirstHiddenReplyIndex == -1 ? page->mFeed.size() : page->mFirstHiddenReplyIndex + 1;
    const size_t pageInsertCount = page->mFirstHiddenReplyIndex == -1 ? page->mFeed.size() : page->mFirstHiddenReplyIndex;

    beginInsertRows({}, 0, newRowCount - 1);
    insertPage(mFeed.end(), *page, pageInsertCount);

    if (page->mFirstHiddenReplyIndex != -1)
        mFeed.push_back(Post::createHiddenPosts());

    if (!mFeed.empty())
        mFeed.back().setEndOfFeed(true);

    endInsertRows();

    if (page->mFirstHiddenReplyIndex != -1)
        mHiddenRepliesFeed.assign(page->mFeed.begin() + page->mFirstHiddenReplyIndex, page->mFeed.end());

    qDebug() << "New feed size:" << mFeed.size();
    return page->mEntryPostIndex;
}

bool PostThreadModel::addMorePosts(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread)
{
    if (mFeed.empty())
    {
        qWarning() << "Cannot add more posts to an empty thread";
        return false;
    }

    setThreadgateView(thread->mThreadgate);
    auto page = createPage(thread, true);

    if (page->mFeed.size() <= 1)
    {
        qWarning() << "Page has no new posts:" << page->mFeed.size();
        return false;
    }

    const auto& post = page->mFeed.front();
    const int index = findPost(post.getCid());

    if (index == -1)
    {
        qWarning() << "Post not found, cid:" << post.getCid();
        return false;
    }

    // NOTE: the first post must not be inserted as it is the same as the leaf post to
    // which we add.
    const size_t pageInsertCount = (page->mFirstHiddenReplyIndex == -1 ? page->mFeed.size() : page->mFirstHiddenReplyIndex) - 1;

    if (pageInsertCount > 0)
    {
        beginInsertRows({}, index + 1, index + pageInsertCount);
        auto feedInsertIt = mFeed.begin() + index + 1;
        mFeed.insert(feedInsertIt, page->mFeed.begin() + 1, page->mFeed.begin() + 1 + pageInsertCount);
        endInsertRows();
    }

    if (page->mFirstHiddenReplyIndex != -1)
    {
        mHiddenRepliesFeed.insert(mHiddenRepliesFeed.end(), page->mFeed.begin() + page->mFirstHiddenReplyIndex, page->mFeed.end());

        if (!mFeed.back().isHiddenPosts())
        {
            beginInsertRows({}, mFeed.size(), mFeed.size());
            mFeed.push_back(Post::createHiddenPosts());
            endInsertRows();
        }
    }

    auto& postBeforeInsert = mFeed[index];
    postBeforeInsert.removeThreadType(QEnums::THREAD_LEAF);
    postBeforeInsert.setEndOfFeed(false);
    mFeed.back().setEndOfFeed(true);
    changeData({ int(Role::PostThreadType), int(Role::EndOfFeed) });

    return true;
}

void PostThreadModel::addOlderPosts(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread)
{
    if (mFeed.empty())
    {
        qWarning() << "Cannot add older posts to an empty thread";
        return;
    }

    setThreadgateView(thread->mThreadgate);
    auto page = createPage(thread, false);

    if (page->mFeed.size() <= 1)
    {
        qWarning() << "Page has no new posts:" << page->mFeed.size();
        return;
    }

    const auto& leafPost = page->mFeed.back();

    if (leafPost.getUri() != getRootUri())
    {
        qWarning() << "Root post mismatch:" << getRootUri() << "leaf:" << leafPost.getUri();
        return;
    }

    // The leaf node must not be inserted as it is the same as the current root
    const size_t pageInsertCount = page->mFeed.size() - 1;
    Q_ASSERT(pageInsertCount > 0);
    mFeed.front().removeThreadType(QEnums::THREAD_TOP);

    beginInsertRows({}, 0, pageInsertCount - 1);
    mFeed.insert(mFeed.begin(), page->mFeed.begin(), page->mFeed.begin() + pageInsertCount);
    endInsertRows();

    mFeed.front().addThreadType(QEnums::THREAD_TOP);
    changeData({ int(Role::PostThreadType) });
}

QString PostThreadModel::getRootUri() const
{
    return !mFeed.empty() ? mFeed.front().getUri() : QString{};
}

QString PostThreadModel::getPostToAttachMore() const
{
    if (mFeed.empty())
        return {};

    const auto& firstPost = mFeed.front();
    const auto authorDid = firstPost.getAuthorDid();

    for (const auto& post : mFeed)
    {
        if (post.getThreadType() & QEnums::THREAD_LEAF)
        {
            if (post.getReplyCount() == 0)
                return {};

            if (post.getAuthorDid() == authorDid)
                return post.getUri();

            return {};
        }

        if (post.getAuthorDid() != authorDid)
            break;
    }

    return {};
}

void PostThreadModel::showHiddenReplies()
{
    qDebug() << "Show hidden replies:" << mHiddenRepliesFeed.size();

    if (mHiddenRepliesFeed.empty())
    {
        qDebug() << "No hidden replies";
        return;
    }

    // Remove hidden replies place holder
    beginRemoveRows({}, mFeed.size() - 1, mFeed.size() - 1);
    mFeed.erase(mFeed.begin() + mFeed.size() - 1);
    endRemoveRows();

    const size_t newRowCount = mFeed.size() + mHiddenRepliesFeed.size();

    beginInsertRows({}, mFeed.size(), newRowCount - 1);
    mFeed.insert(mFeed.end(), mHiddenRepliesFeed.begin(), mHiddenRepliesFeed.end());
    mFeed.back().setEndOfFeed(true);
    endInsertRows();

    mHiddenRepliesFeed.clear();
    qDebug() << "New feed size:" << mFeed.size();
}

void PostThreadModel::unrollThread()
{
    qDebug() << "Unroll thread, feed size:" << mFeed.size();

    if (mFeed.empty())
        return;

    if (!mUnrollThread)
        return;

    mFirstPostFromUnrolledThread = mFeed.front();
    const auto unrolledFeed = ThreadUnroller::unrollThread(mFeed);
    qDebug() << "Unrolled feeds szie:" << unrolledFeed.size();

    beginRemoveRows({}, 0, mFeed.size() - 1);
    mFeed.clear();
    endRemoveRows();

    beginInsertRows({}, 0, unrolledFeed.size() - 1);
    mFeed = unrolledFeed;
    endInsertRows();
}

QString PostThreadModel::getFirstUnrolledPostText() const
{
    if (!mFirstPostFromUnrolledThread)
        return "";

    return mFirstPostFromUnrolledThread->getFormattedText();
}

QString PostThreadModel::getFirstUnrolledPostPlainText() const
{
    if (!mFirstPostFromUnrolledThread)
        return "";

    return mFirstPostFromUnrolledThread->getText();
}

QString PostThreadModel::getFullThreadPlainText() const
{
    if (mFeed.empty())
        return "";

    QString text = mFeed.front().getText();

    for (int i = 1; i < (int)mFeed.size(); ++i)
    {
        const Post& post = mFeed[i];
        text += '\n';
        text += post.getText();
    }

    return text;
}

QEnums::ReplyRestriction PostThreadModel::getReplyRestriction() const
{
    if (mFeed.empty())
        return QEnums::REPLY_RESTRICTION_NONE;

    const auto& post = mFeed[0];
    const auto* change = getLocalChange(post.getCid());
    const auto restriction = change && change->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? change->mReplyRestriction : post.getReplyRestriction();

    if (post.isReplyDisabled() && restriction == QEnums::REPLY_RESTRICTION_NONE)
        return QEnums::REPLY_RESTRICTION_UNKNOWN;

    return restriction;
}

BasicProfile PostThreadModel::getReplyRestrictionAuthor() const
{
    if (mFeed.empty())
        return {};

    const auto& post = mFeed[0];
    const auto* change = getLocalChange(post.getCid());
    const auto restriction = change && change->mReplyRestriction != QEnums::REPLY_RESTRICTION_UNKNOWN ? change->mReplyRestriction : post.getReplyRestriction();

    if (restriction == QEnums::REPLY_RESTRICTION_NONE)
        return {};

    return post.getAuthor();
}

ListViewBasicList PostThreadModel::getReplyRestrictionLists() const
{
    if (mFeed.empty())
        return {};

    const auto& post = mFeed[0];
    const auto* change = getLocalChange(post.getCid());
    return change && change->mReplyRestrictionLists ? *change->mReplyRestrictionLists : post.getReplyRestrictionLists();
}

void PostThreadModel::clear()
{
    if (!mFeed.empty())
    {
        beginRemoveRows({}, 0, mFeed.size() - 1);
        clearFeed();
        endRemoveRows();
    }

    setThreadgateView(nullptr);
    mHiddenRepliesFeed.clear();
    qDebug() << "All posts removed";
}

static void cacheAuthor(const Post& post)
{
    const auto* postView = post.getPostView();
    if (postView)
    {
        const auto& author = postView->mAuthor;
        if (author)
        {
            const BasicProfile authorProfile(author);
            AuthorCache::instance().put(authorProfile);
        }
    }
}

Post& PostThreadModel::Page::addPost(const Post& post)
{
    mPostFeedModel.preprocess(post);
    mFeed.push_back(post);
    mFeed.back().setPostType(QEnums::POST_THREAD);
    cacheAuthor(post);
    return mFeed.back();
}

Post& PostThreadModel::Page::prependPost(const Post& post)
{
    mPostFeedModel.preprocess(post);
    mFeed.push_front(post);
    mFeed.front().setPostType(QEnums::POST_THREAD);
    cacheAuthor(post);
    return mFeed.front();
}

void PostThreadModel::Page::addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply,
                                           bool directReply, bool firstDirectReply, int indentLevel)
{
    auto threadPost = Post::createPost(reply, mPostFeedModel.mThreadgateView);
    threadPost.addThreadType(QEnums::THREAD_CHILD);
    threadPost.setThreadIndentLevel(indentLevel);

    if (directReply)
    {
        threadPost.addThreadType(QEnums::THREAD_DIRECT_CHILD);

        if (firstDirectReply)
            threadPost.addThreadType(QEnums::THREAD_FIRST_DIRECT_CHILD);
    }
    else
    {
        threadPost.setParentInThread(true);
    }

    addPost(threadPost);

    if (!threadPost.isPlaceHolder())
    {
        const auto& post(std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(reply.mPost));
        if (!post->mReplies.empty())
        {
            mPostFeedModel.sortReplies(post.get());
            const auto& nextReply = post->mReplies[0];
            Q_ASSERT(nextReply);

            // Hide a reply that is not a direct reply of the thread entry post.
            // The user will see the current post as a post with a non-zero reply count.
            // By clicking on this post the hidden replies can be accessed.
            if (!mPostFeedModel.isHiddenReply(*nextReply))
            {
                if (mPostFeedModel.mOnlyEntryAuthorPosts)
                {
                    auto nextReplyPost = Post::createPost(*nextReply, mPostFeedModel.mThreadgateView);

                    if (nextReplyPost.getAuthorDid() == threadPost.getAuthorDid())
                        addReplyThread(*nextReply, false, false, indentLevel);
                }
                else
                {
                    addReplyThread(*nextReply, false, false, indentLevel);
                }
            }
            else
            {
                mFeed.back().addThreadType(QEnums::THREAD_LEAF);
            }
        }
        else
        {
            mFeed.back().addThreadType(QEnums::THREAD_LEAF);
        }
    }
    else
    {
        mFeed.back().addThreadType(QEnums::THREAD_LEAF);
    }
}

void PostThreadModel::setThreadgateView(const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView)
{
    mThreadgateView = threadgateView;
}

bool PostThreadModel::isHiddenReply(const QString& uri) const
{
    if (!mThreadgateView || !mThreadgateView->mRecord)
        return false;

    return mThreadgateView->mRecord->mHiddenReplies.contains(uri);
}

bool PostThreadModel::isHiddenReply(const ATProto::AppBskyFeed::ThreadElement& reply) const
{
    if (reply.mType != ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
        return false;

    const auto post = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(reply.mPost).get();
    Q_ASSERT(post);
    return isHiddenReply(post->mPost->mUri);
}

// Pin posts are used to fill the bookmark feed. They are annoying in the thread.
bool PostThreadModel::isPinPost(const ATProto::AppBskyFeed::PostView& post) const
{
    if (post.mRecordType != ATProto::RecordType::APP_BSKY_FEED_POST)
        return false;

    const auto postRecord = std::get<ATProto::AppBskyFeed::Record::Post::SharedPtr>(post.mRecord).get();
    Q_ASSERT(postRecord);
    return postRecord->mText == "📌";
}

void PostThreadModel::sortReplies(ATProto::AppBskyFeed::ThreadViewPost* viewPost) const
{
    std::sort(viewPost->mReplies.begin(), viewPost->mReplies.end(),
        [this, viewPost](const ATProto::AppBskyFeed::ThreadElement::SharedPtr& lhs, const ATProto::AppBskyFeed::ThreadElement::SharedPtr& rhs) {
            // THREAD_VIEW_POST before others
            if (lhs->mType != rhs->mType)
            {
                if (lhs->mType == ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
                    return true;

                if (rhs->mType == ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
                    return false;

                return lhs->mType < rhs->mType;
            }

            if (lhs->mType != ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
                return lhs < rhs; // no order here, just pick pointer order

            Q_ASSERT(lhs->mType == ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST);
            Q_ASSERT(rhs->mType == ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST);

            const auto& lhsPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(lhs->mPost)->mPost;
            const auto& rhsPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(rhs->mPost)->mPost;

            // Non-hidden before hidden
            const bool lhsHidden = isHiddenReply(lhsPost->mUri);
            const bool rhsHidden = isHiddenReply(rhsPost->mUri);

            if (lhsHidden != rhsHidden)
                return lhsHidden < rhsHidden;

            switch (mReplyOrder)
            {
            case QEnums::REPLY_ORDER_SMART:
                return smartLessThan(viewPost, lhsPost, rhsPost);
            case QEnums::REPLY_ORDER_OLDEST_FIRST:
                return olderLessThan(viewPost, lhsPost, rhsPost);
            case QEnums::REPLY_ORDER_NEWEST_FIRST:
                return newerLessThan(viewPost, lhsPost, rhsPost);
            }

            qWarning() << "Unknown reply order:" << mReplyOrder;
            return smartLessThan(viewPost, lhsPost, rhsPost);
        });
}

// Sort replies in this order:
// 1. Reply from author
// 2. Your replies
// 3. Replies from following
// 4. Replies from other
// 5. Hidden replies (previous steps only for non-hidden replies)
// In each group, new before old.
bool PostThreadModel::smartLessThan(ATProto::AppBskyFeed::ThreadViewPost* viewPost,
                                    ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                                    ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const
{
    // Non-pin before pin
    const bool lhsPin = isPinPost(*lhsReply);
    const bool rhsPin = isPinPost(*rhsReply);

    if (lhsPin != rhsPin)
        return lhsPin < rhsPin;

    const auto& lhsDid = lhsReply->mAuthor->mDid;
    const auto& rhsDid = rhsReply->mAuthor->mDid;

    if (lhsDid != rhsDid)
    {
        // Author before others
        if (lhsDid == viewPost->mPost->mAuthor->mDid)
            return true;

        if (rhsDid == viewPost->mPost->mAuthor->mDid)
            return false;

        // User before others
        if (lhsDid == mUserDid)
            return true;

        if (rhsDid == mUserDid)
            return false;

        const bool lhsFollowing = mFollowing.contains(lhsDid);
        const bool rhsFollowing = mFollowing.contains(rhsDid);

        // Following before non-following
        if (lhsFollowing != rhsFollowing)
            return lhsFollowing;

        // New before old
        return lhsReply->mIndexedAt > rhsReply->mIndexedAt;
    }

    // When we unroll a thread we filter out all posts from the same author.
    // If the author made multiple replies on a post, then we want the oldest,
    // assuming that the thread was posted in one go, the oldest is most likely
    // the thread continuation.
    if (mOnlyEntryAuthorPosts)
        return lhsReply->mIndexedAt < rhsReply->mIndexedAt;

    // New before old
    return lhsReply->mIndexedAt > rhsReply->mIndexedAt;
}

bool PostThreadModel::newerLessThan(ATProto::AppBskyFeed::ThreadViewPost*,
                                    ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                                    ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const
{
    // When we unroll a thread we filter out all posts from the same author.
    // If the author made multiple replies on a post, then we want the oldest,
    // assuming that the thread was posted in one go, the oldest is most likely
    // the thread continuation.
    if (mOnlyEntryAuthorPosts)
        return lhsReply->mIndexedAt < rhsReply->mIndexedAt;

    // New before old
    return lhsReply->mIndexedAt > rhsReply->mIndexedAt;
}

bool PostThreadModel::olderLessThan(ATProto::AppBskyFeed::ThreadViewPost*,
                                    ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                                    ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const
{
    return lhsReply->mIndexedAt < rhsReply->mIndexedAt;
}

PostThreadModel::Page::Ptr PostThreadModel::createPage(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread, bool addMore)
{
    auto page = std::make_unique<Page>(*this);
    page->mRawThread = thread;

    ATProto::AppBskyFeed::ThreadViewPost* viewPost = nullptr;
    const auto& postThread = page->mRawThread->mThread;
    Post post = Post::createPost(*postThread, mThreadgateView);
    post.addThreadType(QEnums::THREAD_ENTRY);
    const auto postEntryDid = post.getAuthorDid();

    if (!post.isPlaceHolder())
    {
        viewPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(postThread->mPost).get();

        if (!viewPost->mParent)
            post.addThreadType(QEnums::THREAD_TOP);
        else
            post.setParentInThread(true);

        if (viewPost->mReplies.empty())
            post.addThreadType(QEnums::THREAD_LEAF);
    }
    else
    {
        post.addThreadType(QEnums::THREAD_TOP);
        post.addThreadType(QEnums::THREAD_LEAF);
    }

    page->addPost(post);

    if (viewPost)
    {
        auto parent = viewPost->mParent.get();

        // If more posts are added to the threads, then parent posts are in the thread model
        // already.
        while (parent && !addMore)
        {
            Post parentPost = Post::createPost(*parent, mThreadgateView);

            if (mOnlyEntryAuthorPosts && parentPost.getAuthorDid() != postEntryDid)
                break;

            parentPost.addThreadType(QEnums::THREAD_PARENT);

            if (!parentPost.isPlaceHolder())
            {
                const auto threadPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(parent->mPost).get();
                parent = threadPost->mParent.get();

                if (!parent)
                    parentPost.addThreadType(QEnums::THREAD_TOP);
                else
                    parentPost.setParentInThread(true);

            }
            else
            {
                parentPost.addThreadType(QEnums::THREAD_TOP);
                parent = nullptr;
            }

            page->prependPost(parentPost);
        }

        // The entry post is now at the end of the feed
        page->mEntryPostIndex = page->mFeed.size() - 1;

        bool firstReply = !addMore;
        const bool directReply = !addMore;
        sortReplies(viewPost);
        const int indentLevel = viewPost->mReplies.size() > 1 ? 1 : 0;

        for (const auto& reply : viewPost->mReplies)
        {
            Q_ASSERT(reply);

            if (mOnlyEntryAuthorPosts)
            {
                Post replyPost = Post::createPost(*reply, mThreadgateView);

                if (replyPost.getAuthorDid() != postEntryDid)
                    continue;
            }

            if (page->mFirstHiddenReplyIndex == -1 && isHiddenReply(*reply))
            {
                page->mFirstHiddenReplyIndex = page->mFeed.size();
                qDebug() << "First hidden reply:" << page->mFirstHiddenReplyIndex;
            }

            page->addReplyThread(*reply, directReply, firstReply, indentLevel);
            firstReply = false;

            // We only need the first reply in a thread if there are multiple by
            // the same author. The other replies are most likely comments on the
            // thread that have been added later.
            if (mOnlyEntryAuthorPosts)
                break;
        }
    }

    return page;
}

QVariant PostThreadModel::getData(int row, AbstractPostFeedModel::Role role)
{
    auto index = createIndex(row, 0);
    return data(index, (int)role);
}

void PostThreadModel::replyRestrictionChanged()
{
    AbstractPostFeedModel::replyRestrictionChanged();
    emit threadReplyRestrictionChanged();
}

void PostThreadModel::replyRestrictionListsChanged()
{
    AbstractPostFeedModel::replyRestrictionListsChanged();
    emit threadReplyRestrictionListsChanged();
}

}
