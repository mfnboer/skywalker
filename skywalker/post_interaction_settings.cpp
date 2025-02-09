// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "post_interaction_settings.h"

namespace Skywalker {

PostInteractionSettings::PostInteractionSettings(const ATProto::AppBskyActor::PostInteractionSettingsPref& settings) :
    mAllowNobody(settings.mRules.mAllowNobody),
    mAllowMention(settings.mRules.mAllowMention),
    mAllowFollower(settings.mRules.mAllowFollower),
    mAllowFollowing(settings.mRules.mAllowFollowing),
    mDisableEmbedding(settings.mDisableEmbedding)
{
    for (const auto& listRule : settings.mRules.mAllowList)
        mAllowListUris.push_back(listRule->mList);
}

bool PostInteractionSettings::getAllowNobody() const
{
    return mAllowNobody;
}

bool PostInteractionSettings::getAllowMention() const
{
    return mAllowMention;
}

bool PostInteractionSettings::getAllowFollower() const
{
    return mAllowFollower;
}

bool PostInteractionSettings::getAllowFollowing() const
{
    return mAllowFollowing;
}

QStringList PostInteractionSettings::getAllowListUris() const
{
    return mAllowListUris;
}

bool PostInteractionSettings::getDisableEmbedding() const
{
    return mDisableEmbedding;
}

}
