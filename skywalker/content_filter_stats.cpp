// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "content_filter_stats.h"

namespace Skywalker {

void ContentFilterStats::clear()
{
    mMutedAuthor = 0;
    mRepostsFromAuthor = 0;

    mHideFromFollowingFeed = 0;
    mLabel = 0;
    mMutedWord = 0;
    mHideFollowingFromFeed = 0;
    mLanguage = 0;
    mQuotesBlockedPost = 0;
    mRepliesFromUnfollowed = 0;
    mRepliesThreadUnfollowed = 0;
    mSelfReposts = 0;
    mFollowingReposts = 0;

    mReplies = 0;
    mReposts = 0;
    mQuotes = 0;

    mContentMode = 0;
}

void ContentFilterStats::report(QEnums::HideReasonType hideReason, const Details& details)
{
    qDebug() << "Report hide reason:" << hideReason;

    switch (hideReason)
    {
    case QEnums::HIDE_REASON_MUTED_AUTHOR:
        ++mMutedAuthor;
        break;
    case QEnums::HIDE_REASON_REPOST_FROM_AUTHOR:
        ++mAuthorsRepostsFromAuthor[std::get<BasicProfile>(details)];
        ++mRepostsFromAuthor;
        break;
    case QEnums::HIDE_REASON_HIDE_FROM_FOLLOWING_FEED:
        ++mHideFromFollowingFeed;
        break;
    case QEnums::HIDE_REASON_LABEL:
        ++mLabel;
        break;
    case QEnums::HIDE_REASON_MUTED_WORD:
        ++mMutedWord;
        break;
    case QEnums::HIDE_REASON_HIDE_FOLLOWING_FROM_FEED:
        ++mHideFollowingFromFeed;
        break;
    case QEnums::HIDE_REASON_LANGUAGE:
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

}
