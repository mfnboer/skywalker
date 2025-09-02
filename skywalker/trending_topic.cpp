// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "trending_topic.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

TrendingTopic::TrendingTopic(const ATProto::AppBskyUnspecced::TrendView::SharedPtr& topic) :
    mTopic(topic)
{
}

TrendingTopic::TrendingTopic(const QString& topic, const QString& link, const QString& category, QEnums::ContentMode contentMode) :
    mTopic{std::make_shared<ATProto::AppBskyUnspecced::TrendView>()},
    mContentMode(contentMode)
{
    mTopic->mTopic = topic;
    mTopic->mDisplayName = topic;
    mTopic->mLink = link;
    mTopic->mCategory = category;
}

QString TrendingTopic::getTopic() const
{
    return mTopic ? mTopic->mDisplayName : "";
}

QString TrendingTopic::getLink() const
{
    if (!mTopic)
        return "";

    QString link = mTopic->mLink;

    if (link.startsWith("/"))
        link = ATProto::ATUri::BSKY_APP_URL + link;

    return link;
}

QDateTime TrendingTopic::getStartedAt() const
{
    return mTopic ? mTopic->mStartedAt : QDateTime{};
}

QEnums::TrendStatus TrendingTopic::getStatus() const
{
    return mTopic ? QEnums::TrendStatus(mTopic->mStatus) : QEnums::TREND_STATUS_UNKNOWN;
}

QString TrendingTopic::getCategory() const
{
    return mTopic ? mTopic->mCategory.value_or("") : "";
}

int TrendingTopic::getPostCount() const
{
    return mTopic ? mTopic->mPostCount : 0;
}

}
