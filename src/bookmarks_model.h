// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "post_cache.h"
#include "presence.h"
#include <atproto/lib/client.h>

namespace Skywalker {

class BookmarksModel : public AbstractPostFeedModel, public Presence
{
    Q_OBJECT
    Q_PROPERTY(bool inProgress READ getInProgress NOTIFY inProgressChanged FINAL)

public:
    using Ptr = std::unique_ptr<BookmarksModel>;

    static constexpr int MAX_PAGE_SIZE = 25; // Max posts in a getPost request

    explicit BookmarksModel(const QString& userDid, const IProfileStore& following,
                            const ContentFilter& contentFilter, const Bookmarks& bookmarks,
                            QObject* parent = nullptr);

    void clear();
    void addBookmarks(const std::vector<QString>& postUris, ATProto::Client& bsky);

    bool getInProgress() const { return mInProgress; }
    void setInProgress(bool inProgress);

signals:
    void failure(QString error);
    void inProgressChanged();

private:
    void addPosts(const std::vector<QString>& postUris);
    void getAuthorsDeletedPosts(const std::vector<QString>& postUris, ATProto::Client& bsky);
    ATProto::AppBskyFeed::PostView* getDeletedPost(const QString& atUri);

    PostCache mPostCache;
    bool mInProgress = false;
    std::unordered_map<QString, ATProto::AppBskyFeed::PostView::Ptr> mDeletedPosts;
};

}
