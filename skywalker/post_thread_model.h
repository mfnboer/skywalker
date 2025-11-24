// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "muted_words.h"

namespace Skywalker {

class PostThreadModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(bool unrollThread READ isUnrollThread CONSTANT FINAL)
    Q_PROPERTY(QEnums::ReplyRestriction replyRestriction READ getReplyRestriction NOTIFY threadReplyRestrictionChanged FINAL)
    Q_PROPERTY(BasicProfile replyRestrictionAuthor READ getReplyRestrictionAuthor NOTIFY threadReplyRestrictionChanged FINAL)
    Q_PROPERTY(ListViewBasicList replyRestrictionLists READ getReplyRestrictionLists NOTIFY threadReplyRestrictionListsChanged FINAL)

public:
    using Ptr = std::unique_ptr<PostThreadModel>;

    explicit PostThreadModel(const QString& threadEntryUri, QEnums::PostThreadType postThreadType,
                             QEnums::ReplyOrder replyOrder,
                             const QString& userDid, const IProfileStore& following,
                             const IProfileStore& mutedReposts,
                             const ContentFilter& contentFilter,
                             const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                             HashtagIndex& hashtags,
                             QObject* parent = nullptr);

    // Returns index of the entry post
    int setPostThread(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread);

    // The entry post of the thread must be an existing leaf node
    bool addMorePosts(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread);

    // The root post of the model must be the leaf of the added thread.
    void addOlderPosts(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread);

    // Get URI of post to which more posts for the natural thread can be attached.
    // The natural thread is a chain of posts from the original author.
    QString getPostToAttachMore() const;

    QEnums::PostThreadType getPostThreadType() const { return mPostThreadType; }
    QString getRootUri() const;
    bool isUnrollThread() const { return mUnrollThread; }
    Q_INVOKABLE QString getThreadEntryUri() const { return mThreadEntryUri; }
    Q_INVOKABLE void showHiddenReplies();

    void unrollThread();
    Q_INVOKABLE QString getFirstUnrolledPostText() const;
    Q_INVOKABLE QString getFirstUnrolledPostPlainText() const;
    Q_INVOKABLE QString getFullThreadPlainText() const;

    // May return UNKNOWN if there are reply restrictions. This will happen
    // if the root is not in the thread, but the first post has replies disabled.
    QEnums::ReplyRestriction getReplyRestriction() const;
    BasicProfile getReplyRestrictionAuthor() const;
    ListViewBasicList getReplyRestrictionLists() const;

    Q_INVOKABLE QVariant getData(int row, AbstractPostFeedModel::Role role);

signals:
    void threadReplyRestrictionChanged();
    void threadReplyRestrictionListsChanged();

protected:
    virtual void replyRestrictionChanged() override;
    virtual void replyRestrictionListsChanged() override;

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;

        explicit Page(PostThreadModel& postFeedModel) : mPostFeedModel(postFeedModel) {}

        std::deque<Post> mFeed;
        ATProto::AppBskyFeed::PostThread::SharedPtr mRawThread;
        int mEntryPostIndex = 0;
        int mFirstHiddenReplyIndex = -1;
        PostThreadModel& mPostFeedModel;

        Post& addPost(const Post& post);
        Post& prependPost(const Post& post);
        void addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply, bool directReply, bool firstDirectReply, int indentLevel);
    };

    void clear();
    void sortReplies(ATProto::AppBskyFeed::ThreadViewPost* viewPost) const;
    bool smartLessThan(ATProto::AppBskyFeed::ThreadViewPost* viewPost,
                       ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                       ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const;
    bool newerLessThan(ATProto::AppBskyFeed::ThreadViewPost* viewPost,
                       ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                       ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const;
    bool olderLessThan(ATProto::AppBskyFeed::ThreadViewPost* viewPost,
                       ATProto::AppBskyFeed::PostView::SharedPtr lhsReply,
                       ATProto::AppBskyFeed::PostView::SharedPtr rhsReply) const;
    Page::Ptr createPage(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread, bool addMore);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);
    void setThreadgateView(const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView);
    bool isHiddenReply(const QString& uri) const;
    bool isHiddenReply(const ATProto::AppBskyFeed::ThreadElement& reply) const;
    bool isPinPost(const ATProto::AppBskyFeed::PostView& post) const;

    ATProto::AppBskyFeed::ThreadgateView::SharedPtr mThreadgateView;
    std::deque<Post> mHiddenRepliesFeed;
    QString mThreadEntryUri;
    bool mUnrollThread = false;
    bool mOnlyEntryAuthorPosts = false;
    QEnums::PostThreadType mPostThreadType;
    std::optional<Post> mFirstPostFromUnrolledThread;
    QEnums::ReplyOrder mReplyOrder = QEnums::REPLY_ORDER_SMART;
};

}
