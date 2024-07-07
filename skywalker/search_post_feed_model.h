// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once

#include "abstract_post_feed_model.h"

namespace Skywalker {

class SearchPostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<SearchPostFeedModel>;

    SearchPostFeedModel(const QString& userDid, const IProfileStore& following,
                        const IProfileStore& mutedReposts,
                        const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                        const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                        HashtagIndex& hashtags,
                        QObject* parent = nullptr);

    // Returns how many entries have been added.
    int setFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);
    int addFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);
    void clear();

    const QString& getCursorNextPage() const { return mCursorNextPage; }

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        void addPost(const Post& post);
    };

    Page::Ptr createPage(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);

    // This must be kept alive as long as there are posts in the feed dependend on it
    std::vector<ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr> mRawFeed;

    QString mCursorNextPage;
};

}
