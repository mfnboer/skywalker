// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "trending_topic.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

TrendingTopic::TrendingTopic(const ATProto::AppBskyUnspecced::TrendingTopic::SharedPtr& topic) :
    mTopic(topic)
{
}

TrendingTopic::TrendingTopic(const QString& topic, const QString& link, QEnums::ContentMode contentMode) :
    mTopic{std::make_shared<ATProto::AppBskyUnspecced::TrendingTopic>()},
    mContentMode(contentMode)
{
    mTopic->mTopic = topic;
    mTopic->mLink = link;
}

QString TrendingTopic::getTopic() const
{
    return mTopic ? mTopic->mTopic : "";
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

}
