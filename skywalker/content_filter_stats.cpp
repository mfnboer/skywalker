// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats.h"

namespace Skywalker {

void ContentFilterStats::clear()
{
    mMutedAuthor = 0;
    mAuthorsMutedAuthor.clear();

    mRepostsFromAuthor = 0;
    mAuthorsRepostsFromAuthor.clear();

    mHideFromFollowingFeed = 0;
    mAuthorsHideFromFollowingFeed.clear();

    mLabel = 0;
    mLabelMap.clear();

    mMutedWord = 0;
    mEntriesMutedWord.clear();

    mHideFollowingFromFeed = 0;

    mLanguage = 0;
    mEntriesLanguage.clear();

    mQuotesBlockedPost = 0;
    mRepliesFromUnfollowed = 0;
    mRepliesThreadUnfollowed = 0;
    mSelfReposts = 0;
    mFollowingReposts = 0;

    mReplies = 0;
    mReposts = 0;
    mQuotes = 0;

    mContentMode = 0;

    mProfileMap.clear();
    mPosts.clear();
    mPostHideInfoMap.clear();
    mCheckedPostCids.clear();
}

void ContentFilterStats::report(const Post& post, QEnums::HideReasonType hideReason, const Details& details)
{
    qDebug() << "Report hide reason:" << hideReason << post.getUri();

    if (mPostHideInfoMap.contains(post.getCid()))
    {
        qDebug() << "Post already reported:" << post.getUri() << post.getCid();
        return;
    }

    mPosts.push_back(post);
    mPostHideInfoMap[post.getCid()] = { hideReason, details };

    switch (hideReason)
    {
    case QEnums::HIDE_REASON_MUTED_AUTHOR:
        if (std::holds_alternative<BasicProfile>(details)) // safety check (should always pass)
            add(std::get<BasicProfile>(details), mAuthorsMutedAuthor);

        ++mMutedAuthor;
        break;
    case QEnums::HIDE_REASON_REPOST_FROM_AUTHOR:
        if (std::holds_alternative<BasicProfile>(details))
            add(std::get<BasicProfile>(details), mAuthorsRepostsFromAuthor);

        ++mRepostsFromAuthor;
        break;
    case QEnums::HIDE_REASON_HIDE_FROM_FOLLOWING_FEED:
        if (std::holds_alternative<BasicProfile>(details))
            add(std::get<BasicProfile>(details), mAuthorsHideFromFollowingFeed);

        ++mHideFromFollowingFeed;
        break;
    case QEnums::HIDE_REASON_LABEL:
        if (std::holds_alternative<ContentLabel>(details))
        {
            const auto contentLabel = std::get<ContentLabel>(details);
            ++mLabelMap[contentLabel.getDid()][contentLabel.getLabelId()];
        }

        ++mLabel;
        break;
    case QEnums::HIDE_REASON_MUTED_WORD:
        if (std::holds_alternative<MutedWordEntry>(details))
            ++mEntriesMutedWord[std::get<MutedWordEntry>(details)];

        ++mMutedWord;
        break;
    case QEnums::HIDE_REASON_HIDE_FOLLOWING_FROM_FEED:
        ++mHideFollowingFromFeed;
        break;
    case QEnums::HIDE_REASON_LANGUAGE:
        if (std::holds_alternative<QString>(details))
            ++mEntriesLanguage[std::get<QString>(details)];

        ++mLanguage;
        break;
    case QEnums::HIDE_REASON_QUOTE_BLOCKED_POST:
        ++mQuotesBlockedPost;
        break;
    case QEnums::HIDE_REASON_REPLY_TO_UNFOLLOWED:
        ++mRepliesFromUnfollowed;
        break;
    case QEnums::HIDE_REASON_REPLY_THREAD_UNFOLLOWED:
        ++mRepliesThreadUnfollowed;
        break;
    case QEnums::HIDE_REASON_SELF_REPOST:
        ++mSelfReposts;
        break;
    case QEnums::HIDE_REASON_FOLLOWING_REPOST:
        ++mFollowingReposts;
        break;
    case QEnums::HIDE_REASON_REPLY:
        ++mReplies;
        break;
    case QEnums::HIDE_REASON_REPOST:
        ++mReposts;
        break;
    case QEnums::HIDE_REASON_QUOTE:
        ++mQuotes;
        break;
    case QEnums::HIDE_REASON_CONTENT_MODE:
        ++mContentMode;
        break;
    case QEnums::HIDE_REASON_NONE:
        qWarning() << "Content not hidden";
        break;
    }
}

void ContentFilterStats::reportChecked(const Post& post)
{
    mCheckedPostCids.insert(post.getCid());
}

static auto profileHandleCompare = [](const ContentFilterStats::ProfileStat& lhs, const QString& rhs)
{
    return lhs.first.getHandle() < rhs;
};

std::vector<ContentFilterStats::ProfileStat> ContentFilterStats::getProfileStats(const DidStatMap& didStatMap) const
{
    std::vector<ContentFilterStats::ProfileStat> result;
    result.reserve(didStatMap.size());

    for (const auto& [did, stat] : didStatMap)
    {
        auto it = mProfileMap.find(did);

        if (it == mProfileMap.end())
        {
            qWarning() << "Profile not found:" << did;
            continue;
        }

        const auto& profile = it->second;
        auto resultIt = std::lower_bound(result.cbegin(), result.cend(), profile.getHandle(), profileHandleCompare);
        result.insert(resultIt, { profile, stat });
    }

    return result;
}

std::vector<ContentFilterStats::ProfileStat> ContentFilterStats::authorsMutedAuthor() const
{
    return getProfileStats(mAuthorsMutedAuthor);
}

std::vector<ContentFilterStats::ProfileStat> ContentFilterStats::authorsRepostsFromAuthor() const
{
    return getProfileStats(mAuthorsRepostsFromAuthor);
}

std::vector<ContentFilterStats::ProfileStat> ContentFilterStats::authorsHideFromFollowingFeed() const
{
    return getProfileStats(mAuthorsHideFromFollowingFeed);
}

void ContentFilterStats::add(const BasicProfile& profile, DidStatMap& didStatMap)
{
    if (!mProfileMap.contains(profile.getDid()))
        mProfileMap[profile.getDid()] = profile;

    ++didStatMap[profile.getDid()];
}

}
