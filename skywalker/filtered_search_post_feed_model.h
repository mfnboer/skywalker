// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "filtered_post_base_model.h"
#include "feed_pager.h"
#include "post_filter.h"

namespace Skywalker {

class SearchPostFeedModel;

class FilteredSearchPostFeedModel : public FilteredPostBaseModel
{
    Q_OBJECT

public:
    using Ptr = std::unique_ptr<FilteredSearchPostFeedModel>;

    explicit FilteredSearchPostFeedModel(IPostFilter::Ptr postFilter,
                                         SearchPostFeedModel* underlyingModel,
                                         const QString& userDid,
                                         const IProfileStore& mutedReposts,
                                         const IContentFilter& contentFilter,
                                         const IMatchWords& mutedWords,
                                         const FocusHashtags& focusHashtags,
                                         HashtagIndex& hashtags,
                                         QObject* parent = nullptr);

    Q_INVOKABLE QVariant getUnderlyingModel();

    void clear();
    void setPosts(const TimelineFeed& posts, size_t numPosts);
    void addPosts(const TimelineFeed& posts, size_t numPosts);

    Q_INVOKABLE void getFeed(IFeedPager* pager);
    Q_INVOKABLE void getFeedNextPage(IFeedPager* pager);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<const Post*> mFeed;

        void addPost(const Post* post);
    };

    Page::Ptr createPage(const TimelineFeed& posts, int startIndex, size_t numPosts);
    void addPage(Page::Ptr page);

    SearchPostFeedModel* mUnderlyingModel = nullptr;
};

}
