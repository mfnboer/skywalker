// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "enums.h"
#include "muted_words.h"
#include "post.h"
#include "profile.h"
#include <deque>

namespace Skywalker {

class PostFeedModel;

class ContentFilterStats
{
public:
    using Details = std::variant<std::nullptr_t, BasicProfile, MutedWordEntry, QString, ContentLabel>;
    using ProfileStat = std::pair<BasicProfile, int>;
    using LabelIdStatMap = std::unordered_map<QString, int>;
    using LabelerDidLabelStatsMap = std::unordered_map<QString, LabelIdStatMap>;

    struct PostHideInfo
    {
        QEnums::HideReasonType mHideReason = QEnums::HIDE_REASON_NONE;
        Details mDetails = nullptr;
    };

    using PostHideInfoMap = std::unordered_map<QString, PostHideInfo>; // cid -> info

    int total() const { return mPosts.size(); }
    int checkedPosts() const { return mCheckedPostCids.size(); }

    int mutedAuthor() const { return mMutedAuthor; }
    std::vector<ProfileStat> authorsMutedAuthor() const;

    int repostsFromAuthor() const { return mRepostsFromAuthor; }
    std::vector<ProfileStat> authorsRepostsFromAuthor() const;

    int hideFromFollowingFeed() const { return mHideFromFollowingFeed; }
    std::vector<ProfileStat> authorsHideFromFollowingFeed() const;

    int label() const { return mLabel; }
    const LabelerDidLabelStatsMap& labelMap() const { return mLabelMap; }

    int mutedWord() const { return mMutedWord; }
    const std::map<MutedWordEntry, int>& entriesMutedWord() const { return mEntriesMutedWord; }

    int hideFollowingFromFeed() const { return mHideFollowingFromFeed; }

    int language() const { return mLanguage; }
    const std::map<QString, int>& entriesLanguage() const { return mEntriesLanguage; }

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
    void report(const Post& post, QEnums::HideReasonType hideReason, const Details& details);
    void reportChecked(const Post& post);

    void setFeed(PostFeedModel* model, QEnums::HideReasonType hideReason) const;

private:
    using DidStatMap = std::unordered_map<QString, int>;

    void add(const BasicProfile& profile, DidStatMap& didStatMap);
    std::vector<ProfileStat> getProfileStats(const DidStatMap& didStatMap) const;
    void addPost(const Post& post);

    int mMutedAuthor = 0;
    DidStatMap mAuthorsMutedAuthor;

    int mRepostsFromAuthor = 0;
    DidStatMap mAuthorsRepostsFromAuthor;

    int mHideFromFollowingFeed = 0;
    DidStatMap mAuthorsHideFromFollowingFeed;

    int mLabel = 0;
    LabelerDidLabelStatsMap mLabelMap;

    int mMutedWord = 0;
    std::map<MutedWordEntry, int> mEntriesMutedWord;

    int mHideFollowingFromFeed = 0;

    int mLanguage = 0;
    std::map<QString, int> mEntriesLanguage;

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
    std::deque<Post> mPosts;
    PostHideInfoMap mPostHideInfoMap;
    std::unordered_set<QString> mCheckedPostCids;
};

}
