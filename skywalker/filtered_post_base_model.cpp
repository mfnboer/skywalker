// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "filtered_post_base_model.h"

namespace Skywalker {

FilteredPostBaseModel::FilteredPostBaseModel(IPostFilter::Ptr postFilter,
                                             const QString& userDid, const IProfileStore& following,
                                             const IProfileStore& mutedReposts,
                                             const IContentFilter& contentFilter,
                                             const IMatchWords& mutedWords,
                                             const FocusHashtags& focusHashtags,
                                             HashtagIndex& hashtags,
                                             QObject* parent) :
    AbstractPostFeedModel(userDid, following, mutedReposts, ProfileStore::NULL_STORE,
                          contentFilter, mutedWords, focusHashtags, hashtags,
                          parent),
    mPostFilter(std::move(postFilter))
{
    Q_ASSERT(mPostFilter);
}

void FilteredPostBaseModel::setCheckedTillTimestamp(QDateTime timestamp)
{
    if (timestamp != mCheckedTillTimestamp)
    {
        mCheckedTillTimestamp = timestamp;
        emit checkedTillTimestampChanged();
    }
}

void FilteredPostBaseModel::setNumPostsChecked(int numPostsChecked)
{
    if (numPostsChecked != mNumPostsChecked)
    {
        mNumPostsChecked = numPostsChecked;
        emit numPostsCheckedChanged();
    }
}


}
