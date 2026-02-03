// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "muted_words.h"
#include <atproto/lib/lexicon/app_bsky_bookmark.h>

namespace Skywalker {

class BookmarksModel : public AbstractPostFeedModel
{
public:
    using Ptr = std::unique_ptr<BookmarksModel>;

    explicit BookmarksModel(const QString& userDid,
                            const IProfileStore& mutedReposts,
                            const ContentFilter& contentFilter,
                            const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                            HashtagIndex& hashtags,
                            QObject* parent = nullptr);

    void clear();
    QString getFeedName() const override { return "bookmarks"; }
    void addBookmarks(const ATProto::AppBskyBookmark::GetBookmarksOutput& output);
    const QString& getCursorNextPage() const { return mCursorNextPage; }

private:
    void addPost(const ATProto::AppBskyFeed::PostView::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr& subject);
    void addPost(const ATProto::AppBskyFeed::NotFoundPost::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr& subject);
    void addPost(const ATProto::AppBskyFeed::BlockedPost::SharedPtr& post, const ATProto::ComATProtoRepo::StrongRef::SharedPtr& subject);

    QString mCursorNextPage;
};

}
