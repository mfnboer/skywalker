// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "filtered_post_base_model.h"
#include "feed_pager.h"
#include "post_filter.h"

namespace Skywalker {

class PostFeedModel;

class FilteredPostFeedModel : public FilteredPostBaseModel
{
    Q_OBJECT
    Q_PROPERTY(QEnums::FeedType feedType READ getFeedType CONSTANT FINAL)

public:
    using Ptr = std::unique_ptr<FilteredPostFeedModel>;

    explicit FilteredPostFeedModel(IPostFilter::Ptr postFilter,
                                   PostFeedModel* underlyingModel,
                                   const QString& userDid,
                                   const IProfileStore& mutedReposts,
                                   const IContentFilter& contentFilter,
                                   const IMatchWords& mutedWords,
                                   const FocusHashtags& focusHashtags,
                                   HashtagIndex& hashtags,
                                   QObject* parent = nullptr);

    Q_INVOKABLE QVariant getUnderlyingModel();
    QEnums::FeedType getFeedType() const;

    void clear();
    void setPosts(const TimelineFeed& posts, size_t numPosts);
    void addPosts(const TimelineFeed& posts, size_t numPosts);
    void prependPosts(const TimelineFeed& posts, size_t numPosts);
    void gapFill(const TimelineFeed& posts, size_t numPosts, int gapId);
    void removeHeadPosts(const TimelineFeed& posts, size_t numPosts);
    void removeTailPosts(const TimelineFeed& posts, size_t numPosts);

    void setEndOfFeed(bool endOfFeed) override;

    Q_INVOKABLE void getFeed(IFeedPager* pager);
    Q_INVOKABLE void getFeedNextPage(IFeedPager* pager);

    Q_INVOKABLE void reportOnScreen(const QString& postUri);
    Q_INVOKABLE void reportOffScreen(const QString& postUri, const QString& feedContext);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<const Post*> mFeed;
        std::unordered_map<int, size_t> mGapIdIndexMap;

        void addPost(const Post* post);
        int addThread(const TimelineFeed& posts, int startIndex, size_t numPosts, int matchedPostIndex);
    };

    Page::Ptr createPage(const TimelineFeed& posts, int startIndex, size_t numPosts);
    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page);
    void addPage(Page::Ptr page);
    void prependPage(Page::Ptr page);
    void removePosts(size_t startIndex, size_t count);
    void addToIndices(int offset, size_t startAtIndex);

    PostFeedModel* mUnderlyingModel = nullptr;

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;
};

}
