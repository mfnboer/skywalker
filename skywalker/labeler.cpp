// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "labeler.h"
#include "content_filter.h"

namespace Skywalker {

LabelerViewerState::LabelerViewerState(const ATProto::AppBskyLabeler::LabelerViewerState::SharedPtr& viewerState) :
    mViewerState(viewerState)
{
}

QString LabelerViewerState::getLike() const
{
    return mViewerState ? mViewerState->mLike.value_or("") : "";
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

LabelerView::LabelerView(const ATProto::AppBskyLabeler::LabelerView::SharedPtr& view) :
    mView(view)
{
}

LabelerView::LabelerView(const ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr& view) :
    mViewDetailed(view)
{
}

QString LabelerView::getUri() const
{
    if (mView)
        return mView->mUri;

    if (mViewDetailed)
        return mViewDetailed->mUri;

    return {};
}

QString LabelerView::getCid() const
{
    if (mView)
        return mView->mCid;

    if (mViewDetailed)
        return mViewDetailed->mCid;

    return {};
}

Profile LabelerView::getCreator() const
{
    if (mView)
        return Profile(mView->mCreator);

    if (mViewDetailed)
        return Profile(mViewDetailed->mCreator);

    return {};
}

int LabelerView::getLikeCount() const
{
    if (mView)
        return mView->mLikeCount;

    if (mViewDetailed)
        return mViewDetailed->mLikeCount;

    return 0;
}

LabelerViewerState LabelerView::getViewer() const
{
    if (mView)
        return LabelerViewerState(mView->mViewer);

    if (mViewDetailed)
        return LabelerViewerState(mViewDetailed->mViewer);

    return {};
}

QDateTime LabelerView::getIndexedAt() const
{
    if (mView)
        return mView->mIndexedAt;

    if (mViewDetailed)
        return mViewDetailed->mIndexedAt;

    return {};
}

ContentLabelList LabelerView::getContentLabels() const
{
    if (mView)
        return ContentFilter::getContentLabels(mView->mLabels);

    if (mViewDetailed)
        return ContentFilter::getContentLabels(mViewDetailed->mLabels);

    return {};
}

LabelerViewDetailed::LabelerViewDetailed(const ATProto::AppBskyLabeler::LabelerViewDetailed::SharedPtr& view) :
    LabelerView(view)
{
}

LabelerPolicies LabelerViewDetailed::getPolicies() const
{
    return mViewDetailed ? LabelerPolicies(*mViewDetailed->mPolicies, getCreator().getDid()) : LabelerPolicies{};
}

}
