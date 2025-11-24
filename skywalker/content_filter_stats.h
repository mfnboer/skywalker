// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include "content_label.h"
#include "enums.h"
#include "muted_words.h"
#include "post.h"
#include "profile.h"
#include <deque>

namespace Skywalker {

class IListStore;
class PostFeedModel;

class ContentFilterStats
{
public:
    using Details = std::variant<std::nullptr_t, BasicProfile, MutedWordEntry, QString, ContentLabel>;
    using ProfileStat = std::pair<BasicProfile, int>;
    using DidStatMap = std::unordered_map<QString, int>;
    using LabelIdStatMap = std::unordered_map<QString, int>;
    using LabelerDidLabelStatMap = std::unordered_map<QString, LabelIdStatMap>;
    using ListUriProfileStatsMap = std::unordered_map<QString, DidStatMap>;
    using ListProfileStat = std::pair<ListViewBasic, std::vector<ProfileStat>>;

    static QString detailsToString(const Details& details, const IContentFilter& contentFilter);

    struct PostHideInfo
    {
        QEnums::HideReasonType mHideReason = QEnums::HIDE_REASON_NONE;
        Details mDetails = nullptr;
    };

    using PostHideInfoMap = std::unordered_map<QString, PostHideInfo>; // cid -> info

    explicit ContentFilterStats(const IListStore& timelineHide);
    ContentFilterStats(const ContentFilterStats&) = default;
    ContentFilterStats& operator=(const ContentFilterStats&) = default;

    int total() const { return mPosts.size(); }
    int checkedPosts() const { return mCheckedPostCids.size(); }

    int mutedAuthor() const { return mMutedAuthor; }
    std::vector<ProfileStat> authorsMutedAuthor() const;

    int repostsFromAuthor() const { return mRepostsFromAuthor; }
    std::vector<ProfileStat> authorsRepostsFromAuthor() const;

    int hideFromFollowingFeed() const { return mHideFromFollowingFeed; }
    std::vector<ListProfileStat> listsHideFromFollowingFeed() const;

    int label() const { return mLabel; }
    const LabelerDidLabelStatMap& labelMap() const { return mLabelMap; }

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

    void setFeed(PostFeedModel* model, QVariantList detailList) const;

private:
    void add(const BasicProfile& profile, DidStatMap& didStatMap);
    void add(const ListViewBasic& list, const BasicProfile& profile, ListUriProfileStatsMap& listUriProfileStatMap);
    void remove(const BasicProfile& profile, DidStatMap& didStatMap);
    void remove(const ListViewBasic& list, const BasicProfile& profile, ListUriProfileStatsMap& listUriProfileStatMap);
    std::vector<ProfileStat> getProfileStats(const DidStatMap& didStatMap) const;
    void addPost(const Post& post);
    void removeLastPost();
    void removeReport(const Post& post, QEnums::HideReasonType hideReason, const Details& details);

    int mMutedAuthor = 0;
    DidStatMap mAuthorsMutedAuthor;

    int mRepostsFromAuthor = 0;
    DidStatMap mAuthorsRepostsFromAuthor;

    int mHideFromFollowingFeed = 0;
    ListUriProfileStatsMap mListsHideFromFollowingFeed;

    int mLabel = 0;
    LabelerDidLabelStatMap mLabelMap;

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

    struct ProfileLink
    {
        BasicProfile mProfile;
        int mCount = 0;
    };

    struct ListLink
    {
        ListViewBasic mList;
        int mCount = 0;
    };

    const IListStore* mTimelineHide = nullptr;
    std::unordered_map<QString, ProfileLink> mProfileMap;
    std::unordered_map<QString, ListLink> mListMap;
    std::deque<Post> mPosts;
    PostHideInfoMap mPostHideInfoMap;
    std::unordered_set<QString> mCheckedPostCids;
};

}
