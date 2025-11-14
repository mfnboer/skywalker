// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats.h"
#include "post_feed_model.h"

namespace Skywalker {

QString ContentFilterStats::detailsToString(const Details& details, const IContentFilter& contentFilter)
{
    if (std::holds_alternative<BasicProfile>(details))
        return "@" + std::get<BasicProfile>(details).getHandle();

    if (std::holds_alternative<MutedWordEntry>(details))
        return std::get<MutedWordEntry>(details).getValue();

    if (std::holds_alternative<ContentLabel>(details))
    {
        const auto& label = std::get<ContentLabel>(details);
        auto* contentGroup = contentFilter.getContentGroup(label.getLabelId(), label.getLabelId());
        return contentGroup ? contentGroup->getTitle() : "";
    }

    if (std::holds_alternative<QString>(details))
        return std::get<QString>(details);

    return {};
}

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

    addPost(post);
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
    case QEnums::HIDE_REASON_ANY:
        qWarning() << "ANY is not a valid reason on a post";
        break;
    }
}

void ContentFilterStats::reportChecked(const Post& post)
{
    mCheckedPostCids.insert(post.getCid());
}

static ContentFilterStats::Details convertLabelDetails(const QVariantList& detailList)
{
    Q_ASSERT(!detailList.empty());

    for (const auto& detail : detailList)
    {
        if (!detail.canConvert<QString>())
        {
            qWarning() << "Invalid hide details for label:" << detailList;
            return nullptr;
        }
    }

    const QString& did = detailList.front().value<QString>();
    const QString labelId = detailList.size() > 1 ? detailList[1].value<QString>() : "";
    return ContentLabel(did, "", "", labelId, {});
}

void ContentFilterStats::setFeed(PostFeedModel* model, QVariantList detailList) const
{
    Q_ASSERT(model);
    if (!model)
        return;

    Details hideDetails = nullptr;

    if (!detailList.empty())
    {
        const auto& detail = detailList.front();

        if (detail.canConvert<BasicProfile>())
            hideDetails = detail.value<BasicProfile>();
        else if (detail.canConvert<MutedWordEntry>())
            hideDetails = detail.value<MutedWordEntry>();
        else if (detail.canConvert<QString>())
        {
            if (model->getHideReason() == QEnums::HIDE_REASON_LABEL)
                hideDetails = convertLabelDetails(detailList);
            else
                hideDetails = detail.value<QString>();
        }
        else
            qWarning() << "Unknown hide detail:" << detail;
    }

    model->setFeed(mPosts, &mPostHideInfoMap, hideDetails);
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

static auto postTimelineCompare = [](const Post& lhs, const Post& rhs)
{
    return lhs.getTimelineTimestamp() > rhs.getTimelineTimestamp();
};

void ContentFilterStats::addPost(const Post& post)
{
    const auto it = std::lower_bound(mPosts.cbegin(), mPosts.cend(), post, postTimelineCompare);
    mPosts.insert(it, post);
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
