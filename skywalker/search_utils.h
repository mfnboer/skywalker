// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "author_list_model.h"
#include "feed_list_model.h"
#include "presence.h"
#include "profile.h"
#include "search_post_feed_model.h"
#include "wrapped_skywalker.h"
#include <vector>

namespace Skywalker {

class SearchUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(BasicProfileList authorTypeaheadList READ getAuthorTypeaheadList WRITE setAuthorTypeaheadList NOTIFY authorTypeaheadListChanged FINAL)
    Q_PROPERTY(QStringList hashtagTypeaheadList READ getHashtagTypeaheadList WRITE setHashtagTypeaheadList NOTIFY hashtagTypeaheadListChanged FINAL)
    Q_PROPERTY(bool searchPostsInProgress READ getSearchPostsInProgress WRITE setSearchPostsInProgress NOTIFY searchPostsInProgressChanged FINAL)
    Q_PROPERTY(bool searchActorsInProgress READ getSearchActorsInProgress WRITE setSearchActorsInProgress NOTIFY searchActorsInProgressChanged FINAL)
    Q_PROPERTY(bool searchFeedsInProgress READ getSearchFeedsInProgress WRITE setSearchFeedsInProgress NOTIFY searchFeedsInProgressChanged FINAL)
    QML_ELEMENT

public:
    static QString normalizeText(const QString& text);
    static std::vector<QString> getNormalizedWords(const QString& text);
    static std::vector<QString> getWords(const QString& text);

    explicit SearchUtils(QObject* parent = nullptr);
    ~SearchUtils();

    Q_INVOKABLE void removeModels();
    Q_INVOKABLE void searchAuthorsTypeahead(const QString& typed, int limit = 20);
    Q_INVOKABLE void searchHashtagsTypeahead(const QString& typed, int limit = 20);
    Q_INVOKABLE void searchPosts(const QString& text, int maxPages = 10, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchPosts(const QString& text, int maxPages = 10, int minEntries = 10);
    Q_INVOKABLE void searchActors(const QString& text, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchActors(const QString& text);
    Q_INVOKABLE void searchFeeds(const QString& text, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchFeeds(const QString& text);
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel();
    Q_INVOKABLE AuthorListModel* getSearchUsersModel();
    Q_INVOKABLE FeedListModel* getSearchFeedsModel();
    Q_INVOKABLE void clearAllSearchResults();

    const BasicProfileList& getAuthorTypeaheadList() const { return mAuthorTypeaheadList; }
    void setAuthorTypeaheadList(const BasicProfileList& list);
    const QStringList& getHashtagTypeaheadList() const { return mHashtagTypeaheadList; }
    void setHashtagTypeaheadList(const QStringList& list);
    bool getSearchPostsInProgress() const { return mSearchPostsInProgress; }
    void setSearchPostsInProgress(bool inProgress);
    bool getSearchActorsInProgress() const { return mSearchActorsInProgress; }
    void setSearchActorsInProgress(bool inProgress);
    bool getSearchFeedsInProgress() const { return mSearchFeedsInProgress; }
    void setSearchFeedsInProgress(bool inProgress);

signals:
    void authorTypeaheadListChanged();
    void hashtagTypeaheadListChanged();
    void searchPostsInProgressChanged();
    void searchActorsInProgressChanged();
    void searchFeedsInProgressChanged();

private:
    void addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList);
    void localSearchAuthorsTypeahead(const QString& typed, int limit);
    QString preProcessSearchText(const QString& text) const;

    BasicProfileList mAuthorTypeaheadList;
    QStringList mHashtagTypeaheadList;
    int mSearchPostFeedModelId = -1;
    int mSearchUsersModelId = -1;
    int mSearchFeedsModelId = -1;
    bool mSearchPostsInProgress = false;
    bool mSearchActorsInProgress = false;
    bool mSearchFeedsInProgress = false;
};

}
