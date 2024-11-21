// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "post_filter.h"

namespace Skywalker {

class FilteredPostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(QDateTime checkedTillTimestamp READ getCheckedTillTimestamp NOTIFY checkedTillTimestampChanged FINAL)

public:
    using Ptr = std::unique_ptr<FilteredPostFeedModel>;

    explicit FilteredPostFeedModel(IPostFilter::Ptr postFilter,
                                   const QString& userDid, const IProfileStore& following,
                                   const IProfileStore& mutedReposts,
                                   const IContentFilter& contentFilter,
                                   const Bookmarks& bookmarks,
                                   const IMatchWords& mutedWords,
                                   const FocusHashtags& focusHashtags,
                                   HashtagIndex& hashtags,
                                   QObject* parent = nullptr);

    QString getFeedName() const { return mPostFilter->getName(); }

    void clear();
    void setPosts(const TimelineFeed& posts, size_t numPosts);
    void addPosts(const TimelineFeed& posts, size_t numPosts);
    void prependPosts(const TimelineFeed& posts, size_t numPosts);
    void gapFill(const TimelineFeed& posts, int gapId);
    void removeHeadPosts(const TimelineFeed& posts, size_t numPosts);
    void removeTailPosts(const TimelineFeed& posts, size_t numPosts);
    void setCheckedTillTimestamp(QDateTime timestamp);
    QDateTime getCheckedTillTimestamp() const { return mCheckedTillTimestamp; }

signals:
    void checkedTillTimestampChanged();

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

    IPostFilter::Ptr mPostFilter;
    QDateTime mCheckedTillTimestamp{QDateTime::currentDateTimeUtc()};

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;
};

}
