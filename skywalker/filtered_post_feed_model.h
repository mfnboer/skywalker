// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "post_feed_model.h"
#include "post_filter.h"

namespace Skywalker {

class FilteredPostFeedModel : public PostFeedModel
{
public:
    explicit FilteredPostFeedModel(const IPostFilter& postFilter,
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
                                   QObject* parent = nullptr);

    void setPosts(const TimelineFeed& posts, const QString& cursor);
    void addPosts(const TimelineFeed& posts, const QString& cursor);
    void prependPosts(const TimelineFeed& posts, const QString& cursor);

private:
    Page::Ptr createPage(const TimelineFeed& posts, const QString& cursor);
    int addThread(const TimelineFeed& posts, int matchedPostIndex, Page& page) const;

    const IPostFilter& mPostFilter;
};

}
