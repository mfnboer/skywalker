// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "labeler.h"

namespace Skywalker {

LabelerViewerState::LabelerViewerState(const ATProto::AppBskyLabeler::LabelerViewerState& viewerState) :
    mLike(viewerState.mLike.value_or(""))
{
}

LabelerPolicies::LabelerPolicies(const ATProto::AppBskyLabeler::LabelerPolicies& policies) :
    mLabelValues(policies.mLabelValues.cbegin(), policies.mLabelValues.cend())
{
    for (const auto& def : policies.mLabelValueDefinitions)
        mLabelContentGroupMap[def->mIdentifier] = ContentGroup(*def);
}

const ContentGroup* LabelerPolicies::getContentGroup(const QString& label) const
{
    auto it = mLabelContentGroupMap.find(label);
    return it != mLabelContentGroupMap.end() ? &it->second : nullptr;
}

LabelerView::LabelerView(const ATProto::AppBskyLabeler::LabelerView& view) :
    mUri(view.mUri),
    mCreator(Profile(view.mCreator.get()).nonVolatileCopy()),
    mLikeCount(view.mLikeCount),
    mViewer(*view.mViewer),
    mIndexedAt(view.mIndexedAt),
    mContentLabels(ContentFilter::getContentLabels(view.mLabels))
{
}

LabelerView::LabelerView(const ATProto::AppBskyLabeler::LabelerViewDetailed& view) :
    mUri(view.mUri),
    mCreator(Profile(view.mCreator.get()).nonVolatileCopy()),
    mLikeCount(view.mLikeCount),
    mViewer(*view.mViewer),
    mIndexedAt(view.mIndexedAt),
    mContentLabels(ContentFilter::getContentLabels(view.mLabels))
{
}

LabelerViewDetailed::LabelerViewDetailed(const ATProto::AppBskyLabeler::LabelerViewDetailed& view) :
    LabelerView(view),
    mPolicies(*view.mPolicies)
{
}

}
