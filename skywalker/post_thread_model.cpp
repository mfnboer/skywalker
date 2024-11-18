// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_thread_model.h"
#include "author_cache.h"

namespace Skywalker {

PostThreadModel::PostThreadModel(const QString& threadEntryUri,
                                 const QString& userDid, const IProfileStore& following,
                                 const IProfileStore& mutedReposts,
                                 const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                                 const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                                 HashtagIndex& hashtags,
                                 QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords, focusHashtags, hashtags, parent),
    mThreadEntryUri(threadEntryUri)
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
    auto page = createPage(thread);

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

    endInsertRows();

    if (page->mFirstHiddenReplyIndex != -1)
        mHiddenRepliesFeed.assign(page->mFeed.begin() + page->mFirstHiddenReplyIndex, page->mFeed.end());

    qDebug() << "New feed size:" << mFeed.size();
    return page->mEntryPostIndex;
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
    endInsertRows();

    mHiddenRepliesFeed.clear();
    qDebug() << "New feed size:" << mFeed.size();
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
                                           bool directReply, bool firstDirectReply)
{
    auto threadPost = Post::createPost(reply, mPostFeedModel.mThreadgateView);
    threadPost.addThreadType(QEnums::THREAD_CHILD);

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
            auto nextReply = post->mReplies[0];
            Q_ASSERT(nextReply);

            // Hide a reply that is not a direct reply of then thread entry post.
            // The user will see the current post as a post with a non-zero reply count.
            // By clicking on this post the hidden replies can be accessed.
            if (!mPostFeedModel.isHiddenReply(*nextReply))
                addReplyThread(*nextReply, false, false);
            else
                mFeed.back().addThreadType(QEnums::THREAD_LEAF);
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

// Sort replies in this order:
// 1. Reply from author
// 2. Your replies
// 3. Replies from following
// 4. Replies from other
// 5. Hidden replies (previous steps only for non-hidden replies)
// In each group, new before old.
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

            // Non-pin before pin
            const bool lhsPin = isPinPost(*lhsPost);
            const bool rhsPin = isPinPost(*rhsPost);

            if (lhsPin != rhsPin)
                return lhsPin < rhsPin;

            const auto& lhsDid = lhsPost->mAuthor->mDid;
            const auto& rhsDid = rhsPost->mAuthor->mDid;

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
                return lhsPost->mIndexedAt > rhsPost->mIndexedAt;
            }

            // New before old
            return lhsPost->mIndexedAt > rhsPost->mIndexedAt;
        });
}

PostThreadModel::Page::Ptr PostThreadModel::createPage(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread)
{
    auto page = std::make_unique<Page>(*this);
    page->mRawThread = thread;

    ATProto::AppBskyFeed::ThreadViewPost* viewPost = nullptr;
    const auto& postThread = page->mRawThread->mThread;
    Post post = Post::createPost(*postThread, mThreadgateView);
    post.addThreadType(QEnums::THREAD_ENTRY);

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
        while (parent)
        {
            Post parentPost = Post::createPost(*parent, mThreadgateView);
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

        bool firstReply = true;
        sortReplies(viewPost);

        for (const auto& reply : viewPost->mReplies)
        {
            Q_ASSERT(reply);

            if (page->mFirstHiddenReplyIndex == -1 && isHiddenReply(*reply))
            {
                page->mFirstHiddenReplyIndex = page->mFeed.size();
                qDebug() << "First hidden reply:" << page->mFirstHiddenReplyIndex;
            }

            page->addReplyThread(*reply, true, firstReply);
            firstReply = false;
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
