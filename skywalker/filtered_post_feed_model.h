// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "post_filter.h"

namespace Skywalker {

class FilteredPostFeedModel : public AbstractPostFeedModel
{
public:
    explicit FilteredPostFeedModel(const IPostFilter& postFilter,
                                   const QString& userDid, const IProfileStore& following,
                                   const IProfileStore& mutedReposts,
                                   const IContentFilter& contentFilter,
                                   const Bookmarks& bookmarks,
                                   const IMatchWords& mutedWords,
                                   const FocusHashtags& focusHashtags,
                                   HashtagIndex& hashtags,
                                   QObject* parent = nullptr);

    QString getFeedName() const { return mPostFilter.getName(); }

    void clear();
    void setPosts(const TimelineFeed& posts, const QString& cursor);
    void addPosts(const TimelineFeed& posts, const QString& cursor);
    void prependPosts(const TimelineFeed& posts, const QString& cursor);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<const Post*> mFeed;
        QString mCursorNextPage;

        void addPost(const Post* post);
        int addThread(const TimelineFeed& posts, int matchedPostIndex);
    };

    Page::Ptr createPage(const TimelineFeed& posts, const QString& cursor);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page);
    void addPage(Page::Ptr page);
    void prependPage(Page::Ptr page);

    const IPostFilter& mPostFilter;
};

}
