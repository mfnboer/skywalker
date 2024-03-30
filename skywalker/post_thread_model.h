// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"

namespace Skywalker {

class PostThreadModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<PostThreadModel>;

    explicit PostThreadModel(const QString& userDid, const IProfileStore& following,
                             const IProfileStore& mutedReposts,
                             const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                             const MutedWords& mutedWords, HashtagIndex& hashtags,
                             QObject* parent = nullptr);

    // Returns index of the entry post
    int setPostThread(ATProto::AppBskyFeed::PostThread::Ptr&& thread);

    // May return UNKNOWN if there are reply restrictions. This will happen
    // if the root is not in the thread, but the first post has replies disabled.
    Q_INVOKABLE QEnums::ReplyRestriction getReplyRestriction() const;
    Q_INVOKABLE BasicProfile getReplyRestrictionAuthor() const;
    Q_INVOKABLE ListViewBasicList getReplyRestrictionLists() const;

    Q_INVOKABLE QVariant getData(int row, AbstractPostFeedModel::Role role);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;

        explicit Page(PostThreadModel& postFeedModel) : mPostFeedModel(postFeedModel) {}

        std::deque<Post> mFeed;
        ATProto::AppBskyFeed::PostThread::Ptr mRawThread;
        int mEntryPostIndex = 0;
        PostThreadModel& mPostFeedModel;

        Post& addPost(const Post& post);
        Post& prependPost(const Post& post);
        void addReplyThread(const ATProto::AppBskyFeed::ThreadElement& reply, bool directReply, bool firstDirectReply);
    };

    void clear();
    void sortReplies(ATProto::AppBskyFeed::ThreadViewPost* viewPost) const;
    Page::Ptr createPage(ATProto::AppBskyFeed::PostThread::Ptr&& thread);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize);

    // This must be kept alive as long as there are posts in the feed dependend on it
    ATProto::AppBskyFeed::PostThread::Ptr mRawThread;
};

}
