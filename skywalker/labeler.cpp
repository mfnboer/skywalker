// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "labeler.h"

namespace Skywalker {

LabelerViewerState::LabelerViewerState(const ATProto::AppBskyLabeler::LabelerViewerState& viewerState) :
    mLike(viewerState.mLike.value_or(""))
{
}

LabelerPolicies::LabelerPolicies(const ATProto::AppBskyLabeler::LabelerPolicies& policies, const QString& did) :
    mLabelValues(policies.mLabelValues.cbegin(), policies.mLabelValues.cend()),
    mLabelerDid(did)
{
    for (const auto& def : policies.mLabelValueDefinitions)
        mLabelContentGroupMap[def->mIdentifier] = ContentGroup(*def, did);
}

const ContentGroup* LabelerPolicies::getContentGroup(const QString& label) const
{
    auto it = mLabelContentGroupMap.find(label);
    return it != mLabelContentGroupMap.end() ? &it->second : nullptr;
}

std::vector<ContentGroup> LabelerPolicies::getContentGroupList() const
{
    std::vector<ContentGroup> groupList;
    groupList.reserve(mLabelValues.size());

    for (const QString& label : mLabelValues)
    {
        if (ContentLabel::isSystemLabelId(label))
        {
            qDebug() << "System label:" << label;
            continue;
        }

        const auto* customGroup = getContentGroup(label);

        if (customGroup)
        {
            qDebug() << "Custom label:" << label;
            groupList.push_back(*customGroup);
            continue;
        }

        const auto* globalGroup = ContentFilter::getGlobalContentGroup(label);

        if (globalGroup)
        {
            qDebug() << "Global label:" << label;
            groupList.push_back(*globalGroup);
            continue;
        }

        qDebug() << "Label without definition:" << label;
        groupList.push_back(ContentGroup(label, mLabelerDid));
    }

    return groupList;
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
    mPolicies(*view.mPolicies, getCreator().getDid())
{
}

}
