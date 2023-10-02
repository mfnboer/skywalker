// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"

namespace Skywalker {

class AuthorFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<AuthorFeedModel>;

    AuthorFeedModel(const QString& author, const QString& userDid, const IProfileStore& following, QObject* parent = nullptr);

    // Returns how many entries have been added.
    int setFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);
    int addFeed(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);

    const QString& getAuthor() const { return mAuthor; }
    const QString& getCursorNextPage() const { return mCursorNextPage; }

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        void addPost(const Post& post);
    };

    void clear();
    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::Ptr&& feed);

    QString mAuthor;

    // This must be kept alive as long as there are posts in the feed dependend on it
    std::vector<ATProto::AppBskyFeed::OutputFeed::Ptr> mRawFeed;

    QString mCursorNextPage;
};

}
