// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
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
    Q_PROPERTY(bool languageFilterConfigured READ isLanguageFilterConfigured NOTIFY languageFilterConfiguredChanged FINAL)
    Q_PROPERTY(bool languageFilterEnabled READ isLanguageFilterEnabled WRITE enableLanguageFilter NOTIFY languageFilterEnabledChanged FINAL)
    Q_PROPERTY(LanguageList filteredLanguages READ getFilterdLanguages NOTIFY languageFilterConfiguredChanged FINAL)
    Q_PROPERTY(bool showPostWithMissingLanguage READ showPostWithMissingLanguage NOTIFY languageFilterConfiguredChanged)

public:
    using Ptr = std::unique_ptr<PostFeedModel>;

    explicit PostFeedModel(const QString& feedName,
                           const QString& userDid, const IProfileStore& following,
                           const IProfileStore& mutedReposts,
                           const IContentFilter& contentFilter,
                           const Bookmarks& bookmarks,
                           const IMatchWords& mutedWords,
                           const FocusHashtags& focusHashtags,
                           HashtagIndex& hashtags,
                           const ATProto::UserPreferences& userPrefs,
                           const UserSettings& userSettings,
                           QObject* parent = nullptr);

    const QString& getFeedName() const { return mFeedName; }

    Q_INVOKABLE const GeneratorView getGeneratorView() const { return mGeneratorView; }
    void setGeneratorView(const GeneratorView& view) { mGeneratorView = view; }

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

    // Get the timestamp of the last post in the feed
    QDateTime lastTimestamp() const;

    // Returns the index of the first post <= timestamp
    int findTimestamp(QDateTime timestamp) const;

    void clear();

    Q_INVOKABLE void unfoldPosts(int startIndex);

    bool hasFilters() const { return !mFilteredPostFeedModels.empty(); }
    Q_INVOKABLE FilteredPostFeedModel* addAuthorFilter(const QString& did, const QString& handle);
    Q_INVOKABLE void deleteFilteredPostFeedModel(FilteredPostFeedModel* postFeedModel);

signals:
    void languageFilterConfiguredChanged();
    void languageFilterEnabledChanged();

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        TimelineFeed mFeed;
        QString mCursorNextPage;
        std::unordered_set<QString> mAddedCids;
        std::unordered_map<QString, int> mParentIndexMap;
        std::unordered_map<QString, ATProto::AppBskyFeed::ThreadgateView::SharedPtr> mRootUriToThreadgate;
        bool mOverlapsWithFeed = false;

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
    void addPageToFilteredPostModel(const Page& page, int pageSize);
    void prependPageToFilteredPostModel(const Page& page, int pageSize);
    void gapFillFilteredPostModel(const Page& page, int gapId);
    void removeHeadFromFilteredPostModel(size_t headSize);
    void removeTailFromFilteredPostModel(size_t tailSize);

    FilteredPostFeedModel* addFilteredPostFeedModel(IPostFilter::Ptr postFilter);

    virtual bool mustHideContent(const Post& post) const override;
    bool passLanguageFilter(const Post& post) const;
    bool mustShowReply(const Post& post, const std::optional<PostReplyRef>& replyRef) const;
    bool mustShowQuotePost(const Post& post) const;
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    Page::Ptr createPage(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr&& feed);

    // Returns gap id if insertion created a gap in the feed.
    int insertFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed, int insertIndex, int fillGapId = 0);

    // Returns an index in the page feed
    std::optional<size_t> findOverlapStart(const Page& page, size_t feedIndex) const;

    // Return an index in mFeed
    std::optional<size_t> findOverlapEnd(const Page& page, size_t feedIndex) const;

    void addToIndices(int offset, size_t startAtIndex);
    void logIndices() const;

    const ATProto::UserPreferences& mUserPreferences;
    const UserSettings& mUserSettings;
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
