// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "abstract_post_feed_model.h"
#include "enums.h"
#include "feed_pager.h"
#include "muted_words.h"

namespace Skywalker {

class AuthorFeedModel : public AbstractPostFeedModel
{
    Q_OBJECT
    Q_PROPERTY(QString feedName READ getFeedName CONSTANT FINAL)
    Q_PROPERTY(QEnums::FeedType feedType READ getFeedType CONSTANT FINAL)
    Q_PROPERTY(QString feedDid READ getFeedDid CONSTANT FINAL)
    Q_PROPERTY(bool feedAcceptsInteractions READ feedAcceptsInteractions CONSTANT FINAL)

public:
    using Ptr = std::unique_ptr<AuthorFeedModel>;

    AuthorFeedModel(const DetailedProfile& author, const QString& userDid, const IProfileStore& following,
                    const IProfileStore& mutedReposts,
                    const ContentFilter& contentFilter,
                    const MutedWords& mutedWords, const FocusHashtags& focusHashtags,
                    HashtagIndex& hashtags,
                    QObject* parent = nullptr);

    Q_INVOKABLE bool isFilterModel() const { return false; }
    Q_INVOKABLE AuthorFeedModel* getUnderlyingModel() { return this; }
    QString getFeedName() const;
    QEnums::FeedType getFeedType() const { return QEnums::FEED_AUTHOR; }
    QString getFeedDid() const { return ""; }
    bool feedAcceptsInteractions() const { return false; }

    void setFilter(QEnums::AuthorFeedFilter filter) { mFilter = filter; }
    QEnums::AuthorFeedFilter getFilter() const { return mFilter; }

    // Returns how many entries have been added.
    int setFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    int addFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    Q_INVOKABLE void setPinnedPost(const QString& uri) { mPinnedPostUri = uri; }
    Q_INVOKABLE void removePinnedPost() { mPinnedPostUri.clear(); }
    void clear();

    const BasicProfile& getAuthor() const { return mAuthor; }
    const QString& getCursorNextPage() const { return mCursorNextPage; }

    Q_INVOKABLE void getFeed(IFeedPager* pager);
    Q_INVOKABLE void getFeedNextPage(IFeedPager* pager);

private:
    struct Page
    {
        using Ptr = std::unique_ptr<Page>;
        std::vector<Post> mFeed;
        void addPost(const Post& post);
    };

    Page::Ptr createPage(ATProto::AppBskyFeed::OutputFeed::SharedPtr&& feed);
    bool mustShow(const Post& post) const;
    bool mustShowReplyContext() const;

    BasicProfile mAuthor;

    QString mCursorNextPage;
    QEnums::AuthorFeedFilter mFilter = QEnums::AUTHOR_FEED_FILTER_POSTS;
    QString mPinnedPostUri;
};

}
