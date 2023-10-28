// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "search_post_feed_model.h"
#include "skywalker.h"
#include <QObject>
#include <vector>

namespace Skywalker {

class SearchUtils : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    Q_PROPERTY(BasicProfileList authorTypeaheadList READ getAuthorTypeaheadList WRITE setAuthorTypeaheadList NOTIFY authorTypeaheadListChanged FINAL)
    QML_ELEMENT

public:
    static QString normalizeText(const QString& text);
    static std::vector<QString> getWords(const QString& text);

    explicit SearchUtils(QObject* parent = nullptr);

    Q_INVOKABLE void searchAuthorsTypeahead(const QString& typed);
    Q_INVOKABLE void searchPosts(const QString& text, int maxPages = 50, int minEntries = 10, const QString& cursor = {});
    Q_INVOKABLE void getNextPageSearchPosts(const QString& text, int maxPages = 50, int minEntries = 10);
    Q_INVOKABLE void legacySearchPosts(const QString& text);
    Q_INVOKABLE SearchPostFeedModel* getSearchPostFeedModel();

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);
    const BasicProfileList& getAuthorTypeaheadList() const { return mAuthorTypeaheadList; }
    void setAuthorTypeaheadList(const BasicProfileList& list);

signals:
    void skywalkerChanged();
    void authorTypeaheadListChanged();

private:
    ATProto::Client* bskyClient();
    void addAuthorTypeaheadList(const ATProto::AppBskyActor::ProfileViewBasicList& profileViewBasicList);
    void localSearchAuthorsTypeahead(const QString& typed);
    void getPosts(const std::vector<QString>& uris);

    Skywalker* mSkywalker = nullptr;
    BasicProfileList mAuthorTypeaheadList;
    SearchPostFeedModel::Ptr mSearchPostFeedModel;
    bool mSearchPostsInProgress = false;
};

}
