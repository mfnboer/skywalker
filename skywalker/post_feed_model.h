// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "feed_pager.h"
#include "filtered_post_feed_model.h"
#include "generator_view.h"
#include "post_filter.h"
#include <atproto/lib/user_preferences.h>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace Skywalker {

class PostFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(QString feedUri READ getFeedUri CONSTANT FINAL)
    Q_PROPERTY(QEnums::FeedType feedType READ getFeedType CONSTANT FINAL)
    Q_PROPERTY(QEnums::ContentMode contentMode READ getContentMode CONSTANT FINAL)
    Q_PROPERTY(bool languageFilterConfigured READ isLanguageFilterConfigured NOTIFY languageFilterConfiguredChanged FINAL)
    Q_PROPERTY(bool languageFilterEnabled READ isLanguageFilterEnabled WRITE enableLanguageFilter NOTIFY languageFilterEnabledChanged FINAL)
    Q_PROPERTY(LanguageList filteredLanguages READ getFilterdLanguages NOTIFY languageFilterConfiguredChanged FINAL)
    Q_PROPERTY(bool showPostWithMissingLanguage READ showPostWithMissingLanguage NOTIFY languageFilterConfiguredChanged FINAL)
    Q_PROPERTY(QList<FilteredPostFeedModel*> filteredPostFeedModels READ getFilteredPostFeedModels NOTIFY filteredPostFeedModelsChanged FINAL)

public:
    using Ptr = std::unique_ptr<PostFeedModel>;

    explicit PostFeedModel(const QString& feedName,
                           const QString& userDid, const IProfileStore& following,
                           const IProfileStore& mutedReposts,
                           const IProfileStore& feedHide,
                           const IContentFilter& contentFilter,
                           const Bookmarks& bookmarks,
                           const IMatchWords& mutedWords,
                           const FocusHashtags& focusHashtags,
                           HashtagIndex& hashtags,
                           const ATProto::UserPreferences& userPrefs,
                           UserSettings& userSettings,
                           QObject* parent = nullptr);

    Q_INVOKABLE bool isFilterModel() const { return false; }
    Q_INVOKABLE PostFeedModel* getUnderlyingModel() { return this; }
    const QString& getFeedName() const { return mFeedName; }
    QString getFeedUri() const;
    QEnums::FeedType getFeedType() const;
    void setIsHomeFeed(bool isHomeFeed) { mIsHomeFeed = isHomeFeed; }
    bool isHomeFeed() const { return mIsHomeFeed; }
    QString getPreferencesFeedKey() const;

    Q_INVOKABLE const GeneratorView getGeneratorView() const { return mGeneratorView; }
    void setGeneratorView(const GeneratorView& view) { mGeneratorView = view; }

    QEnums::ContentMode getContentMode() const { return mGeneratorView.getContentMode(); }

    Q_INVOKABLE const ListViewBasic getListView() const { return mListView; }
    void setListView(const ListViewBasic& view) { mListView = view; }

    const QString& getQuoteUri() const { return mQuoteUri; }
    void setQuoteUri(const QString& quoteUri) { mQuoteUri = quoteUri; }

    bool isLanguageFilterConfigured() const; // atproto language filtering
    void enableLanguageFilter(bool enabled); // local language filtering
    bool isLanguageFilterEnabled() const { return mLanguageFilterEnabled; }
    LanguageList getFilterdLanguages() const;
    bool showPostWithMissingLanguage() const;

    void setFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    void addFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);

    // Returns gap id if prepending created a gap in the feed.
    // Returns 0 otherwise.
    int prependFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);

    void setFeed(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed);
    void addFeed(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed);

    // Returns new gap id if the gap was not fully filled, i.e. there is a new gap.
    // Returns 0 otherwise.
    int gapFillFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed, int gapId);

    void removeTailPosts(int size);
    void removeHeadPosts(int size);
    void removePosts(int startIndex, int size);

    QString getLastCursor() const;
    const Post* getGapPlaceHolder(int gapId) const;
    void clearLastInsertedRowIndex() { mLastInsertedRowIndex = -1; }
    int getLastInsertedRowIndex() const { return mLastInsertedRowIndex; }

    // Clear model and filter models
    void clear();

    // Clear model and destroy filter models
    void reset();

    void setGetFeedInProgress(bool inProgress) override;
    void setFeedError(const QString& error) override;
    Q_INVOKABLE void getFeed(IFeedPager* pager);
    Q_INVOKABLE void getFeedNextPage(IFeedPager* pager);

    bool hasFilters() const { return !mFilteredPostFeedModels.empty(); }
    Q_INVOKABLE FilteredPostFeedModel* addAuthorFilter(const BasicProfile& profile);
    Q_INVOKABLE FilteredPostFeedModel* addHashtagFilter(const QString& hashtag);
    Q_INVOKABLE FilteredPostFeedModel* addFocusHashtagFilter(FocusHashtagEntry* focusHashtag);
    Q_INVOKABLE FilteredPostFeedModel* addVideoFilter();
    Q_INVOKABLE FilteredPostFeedModel* addMediaFilter();
    Q_INVOKABLE FilteredPostFeedModel::Ptr deleteFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel);
    Q_INVOKABLE void reorderFilteredPostFeedModels(const QList<FilteredPostFeedModel*>& models);
    QList<FilteredPostFeedModel*> getFilteredPostFeedModels() const;
    Q_INVOKABLE void addFilteredPostFeedModelsFromSettings();

    void refreshAllData() override;
    void refreshAllFilteredModels();
    void makeLocalFilteredModelChange(const std::function<void(LocalProfileChanges*)>& update);
    void makeLocalFilteredModelChange(const std::function<void(LocalPostModelChanges*)>& update);

signals:
    void languageFilterConfiguredChanged();
    void languageFilterEnabledChanged();
    void filteredPostFeedModelsChanged();
    void filteredPostFeedModelAboutToBeAdded();
    void filteredPostFeedModelAdded(FilteredPostFeedModel*);
    void filteredPostFeedModelAboutToBeDeleted(int index);
    void filteredPostFeedModelDeleted(int index);
    void filteredPostFeedModelUpdated(int index);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        TimelineFeed mFeed;
        QString mCursorNextPage;
        std::unordered_set<QString> mAddedCids;
        std::unordered_map<QString, int> mParentIndexMap;
        std::unordered_map<QString, ATProto::AppBskyFeed::ThreadgateView::SharedPtr> mRootUriToThreadgate;
        QDateTime mOldestDiscaredTimestamp;

        void addPost(const Post& post, bool isParent = false);
        bool cidAdded(const QString& cid) const { return mAddedCids.count(cid); }
        bool tryAddToExistingThread(const Post& post, const PostReplyRef& replyRef);
        void collectThreadgate(const Post& post);
        void setThreadgates();
        void foldThreads();
        void foldPosts(int startIndex, int endIndex);
    };

    void insertPage(const TimelineFeed::iterator& feedInsertIt, const Page& page, int pageSize, int fillGapId = 0);
    void addPage(Page::Ptr page);

    void addPageToFilteredPostModels(const Page& page, int pageSize);
    void prependPageToFilteredPostModels(const Page& page, int pageSize);
    void gapFillFilteredPostModels(const Page& page, int pageSize, int gapId);
    void removeHeadFromFilteredPostModels(size_t headSize);
    void removeTailFromFilteredPostModels(size_t tailSize);
    void clearFilteredPostModels();
    void setEndOfFeedFilteredPostModels(bool endOfFeed);

    FilteredPostFeedModel* addFilteredPostFeedModel(IPostFilter::Ptr postFilter);
    void addFilteredPostFeedModel(FilteredPostFeedModel::Ptr model);
    QJsonObject filteredPostFeedModelsToJson();
    int findFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel) const;
    int findFilteredPostFeedModelByFilter(IPostFilter* filter) const;
    void  addFilteredPostFeedModelsFromJson(const QJsonObject& json);
    bool equalModels(QList<FilteredPostFeedModel*> models) const;

    virtual bool mustHideContent(const Post& post) const override;
    bool passLanguageFilter(const Post& post) const;
    bool mustShowReply(const Post& post, const std::optional<PostReplyRef>& replyRef) const;
    bool mustShowQuotePost(const Post& post) const;
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    Page::Ptr createPage(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed);

    // Returns gap id if insertion created a gap in the feed.
    int insertFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed, int insertIndex, int fillGapId = 0);

    // Returns an index in the page feed and a boolean indicating if there was an overlap on discarded posts.
    std::tuple<std::optional<size_t>, bool> findOverlapStart(const Page& page, size_t feedIndex) const;

    // Return an index in mFeed
    std::optional<size_t> findOverlapEnd(const Page& page, size_t feedIndex) const;

    void addToIndices(int offset, size_t startAtIndex);
    void logIndices() const;

    bool mIsHomeFeed = false;
    const ATProto::UserPreferences& mUserPreferences;
    UserSettings& mUserSettings;
    bool mLanguageFilterEnabled = false;

    // The index is the last (non-filtered) post from a received page. The cursor is to get
    // the next page.
    std::map<size_t, QString> mIndexCursorMap; // cursor to post at next index

    // Index of each gap
    std::unordered_map<int, size_t> mGapIdIndexMap;

    int mLastInsertedRowIndex = -1;
    QString mFeedName;
    GeneratorView mGeneratorView;
    ListViewBasic mListView;
    QString mQuoteUri; // posts quoting this post

    std::vector<FilteredPostFeedModel::Ptr> mFilteredPostFeedModels;
};

}
