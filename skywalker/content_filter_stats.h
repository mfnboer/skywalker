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
    using ProfileStat = std::pair<BasicProfile, int>;

    int mutedAuthor() const { return mMutedAuthor; }
    std::vector<ProfileStat> authorsMutedAuthor() const;

    int repostsFromAuthor() const { return mRepostsFromAuthor; }
    std::vector<ProfileStat> authorsRepostsFromAuthor() const;

    int hideFromFollowingFeed() const { return mHideFromFollowingFeed; }
    std::vector<ProfileStat> authorsHideFromFollowingFeed() const;

    int label() const { return mLabel; }
    int mutedWord() const { return mMutedWord; }
    int hideFollowingFromFeed() const { return mHideFollowingFromFeed; }
    int language() const { return mLanguage; }
    int quotesBlockedPost() const { return mQuotesBlockedPost; }
    int repliesFromUnfollowed() const { return mRepliesFromUnfollowed; }
    int repliesThreadUnfollowed() const { return mRepliesThreadUnfollowed; }
    int selfReposts() const { return mSelfReposts; }
    int followingReposts() const { return mFollowingReposts; }

    int replies() const { return mReplies; }
    int reposts() const { return mReposts; }
    int quotes() const { return mQuotes; }

    int contentMode() const { return mContentMode; }

    void clear();
    void report(QEnums::HideReasonType hideReason, const Details& details);

private:
    using DidStatMap = std::unordered_map<QString, int>;

    void add(const BasicProfile& profile, DidStatMap& didStatMap);
    std::vector<ProfileStat> getProfileStats(const DidStatMap& didStatMap) const;

    int mMutedAuthor = 0;
    DidStatMap mAuthorsMutedAuthor;

    int mRepostsFromAuthor = 0;
    DidStatMap mAuthorsRepostsFromAuthor;

    int mHideFromFollowingFeed = 0;
    DidStatMap mAuthorsHideFromFollowingFeed;

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

    std::unordered_map<QString, BasicProfile> mProfileMap;
};

}
