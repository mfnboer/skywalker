// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "filtered_post_feed_model.h"

namespace Skywalker {

FilteredPostFeedModel::FilteredPostFeedModel(const IPostFilter& postFilter,
                                             const QString& feedName,
                                             const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const Bookmarks& bookmarks,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             const ATProto::UserPreferences& userPrefs,
                                             const UserSettings& userSettings,
                                             QObject* parent) :
    PostFeedModel(feedName, userDid, following, mutedReposts, contentFilter, bookmarks, mutedWords,
                  focusHashtags, hashtags, userPrefs, userSettings, parent),
    mPostFilter(postFilter)
{}

void FilteredPostFeedModel::setPosts(const TimelineFeed& posts, const QString& cursor)
{
    clear();
    addPosts(posts, cursor);
}

void FilteredPostFeedModel::addPosts(const TimelineFeed& posts, const QString& cursor)
{
    qDebug() << "Add posts:" << getFeedName() << "posts:" << posts.size() << "cursor:" << cursor;
    auto page = createPage(posts, cursor);

    if (page->mFeed.empty())
    {
        qDebug() << "No posts:" << getFeedName();
        return;
    }

    addPage(std::move(page));
}

void FilteredPostFeedModel::prependPosts(const TimelineFeed& posts, const QString& cursor)
{
    qDebug() << "Prepend posts:" << getFeedName() << "posts:" << posts.size() << "cursor:" << cursor;
    auto page = createPage(posts, cursor);

    if (page->mFeed.empty())
    {
        qDebug() << "No posts:" << getFeedName();
        return;
    }

    prependPage(std::move(page));
}

PostFeedModel::Page::Ptr FilteredPostFeedModel::createPage(const TimelineFeed& posts, const QString& cursor)
{
    auto page = std::make_unique<Page>();
    page->mCursorNextPage = cursor;

    for (int i = 0; i < (int)posts.size(); ++i)
    {
        const auto& post = posts[i];

        if (!mPostFilter.match(post))
            continue;

        if (post.getPostType() == QEnums::POST_STANDALONE)
        {
            page->addPost(post);
            continue;
        }

        i = addThread(posts, i, *page);
    }

    return page;
}

int FilteredPostFeedModel::addThread(const TimelineFeed& posts, int matchedPostIndex, Page& page) const
{
    int startThread = matchedPostIndex;
    for (; startThread >= 0; --startThread)
    {
        if (posts[startThread].getPostType() == QEnums::POST_STANDALONE)
        {
            ++startThread;
            break;
        }
    }

    int endThread = matchedPostIndex;
    for (; endThread < (int)posts.size(); ++endThread)
    {
        if (posts[endThread].getPostType() == QEnums::POST_STANDALONE)
        {
            --endThread;
            break;
        }
    }

    for (int j = startThread; j <= endThread; ++j)
        page.addPost(posts[j]);

    return endThread;
}

}
