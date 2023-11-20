// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
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
    Q_PROPERTY(bool searchPostsInProgress READ getSearchPostsInProgress WRITE setSearchPostsInProgress NOTIFY searchPostsInProgressChanged FINAL)
    Q_PROPERTY(bool searchActorsInProgress READ getSearchActorsInProgress WRITE setSearchActorsInProgress NOTIFY searchActorsInProgressChanged FINAL)
    QML_ELEMENT

public:
    static QString normalizeText(const QString& text);
    static std::vector<QString> getWords(const QString& text);

    explicit SearchUtils(QObject* parent = nullptr);
    ~SearchUtils();

    Q_INVOKABLE void removeModels();
    Q_INVOKABLE void searchAuthorsTypeahead(const QString& typed, int limit = 20);
    Q_INVOKABLE void searchPosts(const QString& text, int maxPages = 50, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchPosts(const QString& text, int maxPages = 50, int minEntries = 10);
    Q_INVOKABLE void legacySearchPosts(const QString& text);
    Q_INVOKABLE void legacySearchActors(const QString& text);
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel();
    Q_INVOKABLE AuthorListModel* getSearchUsersModel();
    Q_INVOKABLE void clearAllSearchResults();

    const BasicProfileList& getAuthorTypeaheadList() const { return mAuthorTypeaheadList; }
    void setAuthorTypeaheadList(const BasicProfileList& list);
    bool getSearchPostsInProgress() const { return mSearchPostsInProgress; }
    void setSearchPostsInProgress(bool inProgress);
    bool getSearchActorsInProgress() const { return mSearchActorsInProgress; }
    void setSearchActorsInProgress(bool inProgress);

signals:
    void authorTypeaheadListChanged();
    void searchPostsInProgressChanged();
    void searchActorsInProgressChanged();

private:
    void addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList);
    void localSearchAuthorsTypeahead(const QString& typed, int limit);
    void getPosts(const std::vector<QString>& uris);
    void getProfiles(const std::vector<QString>& users);

    BasicProfileList mAuthorTypeaheadList;
    int mSearchPostFeedModelId = -1;
    int mSearchUsersModelId = -1;
    bool mSearchPostsInProgress = false;
    bool mSearchActorsInProgress = false;
};

}