// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "author_list_model.h"
#include "feed_list_model.h"
#include "presence.h"
#include "profile.h"
#include "profile_matcher.h"
#include "search_feed.h"
#include "search_post_feed_model.h"
#include "starter_pack_list_model.h"
#include "trending_topic_list_model.h"
#include "wrapped_skywalker.h"
#include <vector>

namespace Skywalker {

class SearchUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(BasicProfileList authorTypeaheadList READ getAuthorTypeaheadList WRITE setAuthorTypeaheadList NOTIFY authorTypeaheadListChanged FINAL)
    Q_PROPERTY(QStringList hashtagTypeaheadList READ getHashtagTypeaheadList WRITE setHashtagTypeaheadList NOTIFY hashtagTypeaheadListChanged FINAL)
    Q_PROPERTY(BasicProfileList lastSearchedProfiles READ getLastSearchedProfiles WRITE setLastSearchedProfiles NOTIFY lastSearchedProfilesChanged FINAL)
    Q_PROPERTY(TrendingTopicListModel* trendingTopicsListModel READ getTrendingTopicsListModel NOTIFY trendingTopicsListModelChanged FINAL)
    Q_PROPERTY(QEnums::ContentVisibility overrideAdultVisibility READ getOverrideAdultVisibility WRITE setOverrideAdultVisibility NOTIFY overrideAdultVisibilityChanged FINAL)
    Q_PROPERTY(int searchPageSize READ getSearchPageSize WRITE setSearchPageSize NOTIFY searchPageSizeChanged FINAL)
    QML_ELEMENT

public:
    static QString normalizeText(const QString& text);
    static int normalizedCompare(const QString& lhs, const QString& rhs);
    static std::vector<QString> getNormalizedWords(const QString& text);
    static std::vector<QString> getWords(const QString& text);
    static std::vector<QString> combineSingleCharsToWords(const std::vector<QString>& words);

    explicit SearchUtils(QObject* parent = nullptr);
    ~SearchUtils();

    Q_INVOKABLE void removeModels();
    Q_INVOKABLE void searchAuthorsTypeahead(const QString& typed, int limit = 20, bool canChatOnly = false);
    Q_INVOKABLE void searchHashtagsTypeahead(const QString& typed, int limit = 20);

    Q_INVOKABLE void searchPosts(const QString& text, const QString& sortOrder,
                                 const QString& author = "", const QString& mentions = "",
                                 const QDateTime& since = {}, bool setSince = false,
                                 const QDateTime& until = {}, bool setUntil = false,
                                 const QString& language = {},
                                 int maxPages = 10, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchPosts(const QString& text, const QString& sortOrder,
                                            const QString& author = "", const QString& mentions = "",
                                            const QDateTime& since = {}, bool setSince = false,
                                            const QDateTime& until = {}, bool setUntil = false,
                                            const QString& language = {},
                                            int maxPages = 10, int minEntries = 10);

    Q_INVOKABLE void syncFeed(const QString& searchQuery, bool sync);
    Q_INVOKABLE void syncSearchPosts(const QString& text,
                                     const QString& author = "", const QString& mentions = "",
                                     const QDateTime& since = {}, bool setSince = false,
                                     const QDateTime& until = {}, bool setUntil = false,
                                     const QString& language = {},
                                     int maxPages = 10);

    Q_INVOKABLE void searchActors(const QString& text, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchActors(const QString& text);

    Q_INVOKABLE void getSuggestedActors(const QString& cursor = {});
    Q_INVOKABLE void getNextPageSuggestedActors();

    Q_INVOKABLE void getSuggestedFollows(const QString& user);
    Q_INVOKABLE void searchFeeds(const QString& text, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchFeeds(const QString& text);
    Q_INVOKABLE void getSuggestedFeeds();
    Q_INVOKABLE void getSuggestedStarterPacks();
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel(const QString& sortOrder, const QString& feedName = "SearchFeed");
    Q_INVOKABLE AuthorListModel* getSearchUsersModel();
    Q_INVOKABLE AuthorListModel* getSearchSuggestedUsersModel();
    Q_INVOKABLE FeedListModel* getSearchFeedsModel();
    Q_INVOKABLE FeedListModel* getSuggestedFeedsModel();
    Q_INVOKABLE StarterPackListModel* getSuggestedStarterPacksModel();
    Q_INVOKABLE void clearAllSearchResults();
    Q_INVOKABLE QStringList getLastSearches() const;
    Q_INVOKABLE void addLastSearch(const QString& search);
    Q_INVOKABLE void removeLastSearch(const QString& search);
    Q_INVOKABLE void addLastSearchedProfile(const BasicProfile& profile);
    Q_INVOKABLE void removeLastSearchedProfile(const QString& profileDid);
    Q_INVOKABLE void clearLastSearches();
    Q_INVOKABLE void initLastSearchedProfiles(bool resolveDids = true);
    Q_INVOKABLE void getTrendingTopics();
    Q_INVOKABLE SearchFeed createSearchFeed(const QString& searchQuery, const QString& authorHandle, const QString& mentionsHandle,
                                            QDateTime since, QDateTime until, const QString& language) const;

    const BasicProfileList& getAuthorTypeaheadList() const { return mAuthorTypeaheadList; }
    void setAuthorTypeaheadList(const BasicProfileList& list);
    const QStringList& getHashtagTypeaheadList() const { return mHashtagTypeaheadList; }
    void setHashtagTypeaheadList(const QStringList& list);
    const BasicProfileList& getLastSearchedProfiles() const { return mLastSearchedProfiles; }
    void setLastSearchedProfiles(const BasicProfileList& list);
    TrendingTopicListModel* getTrendingTopicsListModel() { return mTrendingTopicsListModel.get(); }
    QEnums::ContentVisibility getOverrideAdultVisibility() const { return mOVerrideAdultVisibility; }
    void setOverrideAdultVisibility(QEnums::ContentVisibility visibility);
    int getSearchPageSize() const { return mSearchPageSize.value_or(0); }
    void setSearchPageSize(int pageSize);

signals:
    void authorTypeaheadListChanged();
    void hashtagTypeaheadListChanged();
    void lastSearchedProfilesChanged();
    void trendingTopicsListModelChanged();
    void overrideAdultVisibilityChanged();
    void searchPageSizeChanged();
    void feedSyncStart(int pages, QDateTime rewindTimestamp);
    void feedSyncProgress(int pages, QDateTime timestamp);
    void feedSyncOk(int index, int offsetY);
    void feedSyncFailed();

private:
    void addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasic::List& profileViewBasicList, const IProfileMatcher& matcher = AnyProfileMatcher{});
    void localSearchAuthorsTypeahead(const QString& typed, int limit, const IProfileMatcher& matcher = AnyProfileMatcher{});
    QString preProcessSearchText(const QString& text) const;
    TrendingTopicListModel& createTrendingTopicsListModel();
    QStringList getLastProfileSearches() const;
    void syncSearchPosts(const QString& text,
                         const QString& author, const QString& mentions,
                         const QDateTime& since, bool setSince,
                         const QDateTime& until, bool setUntil,
                         const QString& language,
                         QDateTime tillTimestamp, const QString& cid,
                         int maxPages = 10, const QString& cursor = {});
    QString processSyncPage(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr feed, SearchPostFeedModel& model, QDateTime tillTimestamp, const QString& cid, int maxPages, const QString& cursor);

    BasicProfileList mAuthorTypeaheadList;
    QStringList mHashtagTypeaheadList;
    BasicProfileList mLastSearchedProfiles;
    std::unordered_map<QString, int> mSearchPostFeedModelId; // sort order -> model id
    int mSearchUsersModelId = -1;
    int mSearchSuggestedUsersModelId = -1;
    int mSearchFeedsModelId = -1;
    int mSuggestedFeedsModelId = -1;
    int mSuggestedStarterPacksModelId = -1;
    AnyProfileMatcher mAnyProfileMatcher;
    CanChatProfileMatcher mCanChatProfileMatcher;
    TrendingTopicListModel::Ptr mTrendingTopicsListModel;
    QEnums::ContentVisibility mOVerrideAdultVisibility = QEnums::CONTENT_VISIBILITY_SHOW;
    std::optional<int> mSearchPageSize;
};

}
