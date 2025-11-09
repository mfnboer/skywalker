// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "profile.h"

namespace Skywalker {

class ContentFilterStats
{
public:
    using Details = std::variant<std::nullptr_t, BasicProfile>;
    using ProfileStats = std::unordered_map<BasicProfile, int, BasicProfile::Hash>;

    int mutedAuthor() const { return mMutedAuthor; }
    int repostsFromAuthor() const { return mRepostsFromAuthor; }
    const ProfileStats& authorsRepostsFromAuthor() const { return mAuthorsRepostsFromAuthor; }

    int repliesFromUnfollowed() const { return mRepliesFromUnfollowed; }
    int repliesThreadUnfollowed() const { return mRepliesThreadUnfollowed; }

    void clear();
    void report(QEnums::HideReasonType hideReason, const Details& details);

private:
    int mMutedAuthor = 0;
    int mRepostsFromAuthor = 0;
    ProfileStats mAuthorsRepostsFromAuthor;

    int mHideFromFollowingFeed = 0;
    int mLabel = 0;
    int mMutedWord = 0;
    int mHideFollowingFromFeed = 0;
    int mLanguage = 0;
    int mQuotesBlockedPost = 0;
    int mRepliesFromUnfollowed = 0;
    int mRepliesThreadUnfollowed = 0;
    int mSelfReposts = 0;
    int mFollowingReposts = 0;

    int mReplies = 0;
    int mReposts = 0;
    int mQuotes = 0;

    int mContentMode = 0;
};

}
