// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "feed_pager.h"
#include "filtered_search_post_feed_model.h"
#include "muted_words.h"

namespace Skywalker {

class SearchPostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QList<FilteredSearchPostFeedModel*> filteredPostFeedModels READ getFilteredPostFeedModels NOTIFY filteredPostFeedModelsChanged FINAL)
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(QEnums::FeedType feedType READ getFeedType CONSTANT FINAL)
    Q_PROPERTY(QString feedDid READ getFeedDid CONSTANT FINAL)
    Q_PROPERTY(bool feedAcceptsInteractions READ feedAcceptsInteractions CONSTANT FINAL)
    Q_PROPERTY(QEnums::ContentMode contentMode READ getContentMode CONSTANT FINAL)

public:
    using Ptr = std::unique_ptr<SearchPostFeedModel>;

    SearchPostFeedModel(const QString& feedName, const QString& userDid,
                        const IProfileStore& mutedReposts,
                        const ContentFilter& contentFilter,
                        const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                        HashtagIndex& hashtags,
                        QObject* parent = nullptr);

    void setReverseFeed(bool reverse) override;

    Q_INVOKABLE bool isFilterModel() const { return false; }
    Q_INVOKABLE SearchPostFeedModel* getUnderlyingModel() { return this; }
    const QString& getFeedName() const { return mFeedName; }
    QEnums::FeedType getFeedType() const { return QEnums::FEED_SEARCH; }
    QString getFeedDid() const { return ""; }
    bool feedAcceptsInteractions() const { return false; }
    QEnums::ContentMode getContentMode() const { return QEnums::CONTENT_MODE_UNSPECIFIED; }

    // Returns how many entries have been added.
    int setFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);
    int addFeed(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);
    Q_INVOKABLE void clear();

    void setGetFeedInProgress(bool inProgress) override;
    void setFeedError(const QString& error) override;
    Q_INVOKABLE void getFeed(IFeedPager* pager);
    Q_INVOKABLE void getFeedNextPage(IFeedPager* pager);

    const QString& getCursorNextPage() const { return mCursorNextPage; }

    Q_INVOKABLE FilteredSearchPostFeedModel* addVideoFilter();
    Q_INVOKABLE FilteredSearchPostFeedModel* addMediaFilter();
    Q_INVOKABLE void deleteFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel);
    QList<FilteredSearchPostFeedModel*> getFilteredPostFeedModels() const;

    void refreshAllData() override;
    void refreshAllFilteredModels();
    void makeLocalFilteredModelChange(const std::function<void(LocalProfileChanges*)>& update);
    void makeLocalFilteredModelChange(const std::function<void(LocalPostModelChanges*)>& update);

signals:
    void filteredPostFeedModelsChanged();
    void firstPage();
    void nextPage();

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        TimelineFeed mFeed;
        void addPost(const Post& post);
    };

    Page::Ptr createPage(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr&& feed);
    void addPageToFilteredPostModels(const Page& page, int pageSize);
    void clearFilteredPostModels();
    void setReverseFeedFilteredPostModels(bool reverse);
    void setEndOfFeedFilteredPostModels(bool endOfFeed);
    FilteredSearchPostFeedModel* addFilteredPostFeedModel(IPostFilter::Ptr postFilter);
    FilteredSearchPostFeedModel::Ptr removeFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel);
    int findFilteredPostFeedModel(FilteredSearchPostFeedModel* postFeedModel) const;

    QString mFeedName;
    QString mCursorNextPage;

    std::vector<FilteredSearchPostFeedModel::Ptr> mFilteredPostFeedModels;
};

}
