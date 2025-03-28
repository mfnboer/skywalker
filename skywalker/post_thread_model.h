// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "muted_words.h"

namespace Skywalker {

class PostThreadModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QEnums::ReplyRestriction replyRestriction READ getReplyRestriction NOTIFY threadReplyRestrictionChanged FINAL)
    Q_PROPERTY(BasicProfile replyRestrictionAuthor READ getReplyRestrictionAuthor NOTIFY threadReplyRestrictionChanged FINAL)
    Q_PROPERTY(ListViewBasicList replyRestrictionLists READ getReplyRestrictionLists NOTIFY threadReplyRestrictionListsChanged FINAL)

public:
    using Ptr = std::unique_ptr<PostThreadModel>;

    explicit PostThreadModel(const QString& threadEntryUri,
                             const QString& userDid, const IProfileStore& following,
                             const IProfileStore& mutedReposts,
                             const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                             const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                             HashtagIndex& hashtags,
                             QObject* parent = nullptr);

    // Returns index of the entry post
    int setPostThread(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread);

    // Then entry posts of the thread must be an existing leaf node
    bool addMorePosts(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread);

    // Get URI of post to which more posts for the natural thread can be attached.
    // The natural thread is a chain of posts from the original author.
    QString getPostToAttachMore() const;

    Q_INVOKABLE QString getThreadEntryUri() const { return mThreadEntryUri; }
    Q_INVOKABLE void showHiddenReplies();

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
    Page::Ptr createPage(const ATProto::AppBskyFeed::PostThread::SharedPtr& thread, bool addMore);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);
    void setThreadgateView(const ATProto::AppBskyFeed::ThreadgateView::SharedPtr& threadgateView);
    bool isHiddenReply(const QString& uri) const;
    bool isHiddenReply(const ATProto::AppBskyFeed::ThreadElement& reply) const;
    bool isPinPost(const ATProto::AppBskyFeed::PostView& post) const;

    ATProto::AppBskyFeed::ThreadgateView::SharedPtr mThreadgateView;
    std::deque<Post> mHiddenRepliesFeed;
    QString mThreadEntryUri;
};

}
