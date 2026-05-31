// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"
#include "content_filter.h"
#include "profile.h"

namespace Skywalker
{

ExternalView::ExternalView(const ATProto::AppBskyEmbed::ExternalViewExternal::SharedPtr& external) :
    mExternal(external)
{}

QString ExternalView::getUri() const
{
    return mExternal ? mExternal->mUri : "";
}

QString ExternalView::getTitle() const
{
    if (!mHtmlTitle.isEmpty())
        return mHtmlTitle;

    return mExternal ? mExternal->mTitle : "";
}

QString ExternalView::getDescription() const
{
    if (!mHtmlDescription.isEmpty())
        return mHtmlDescription;

    return mExternal ? mExternal->mDescription : "";
}

QString ExternalView::getThumbUrl() const
{
    return mExternal ? mExternal->mThumb.value_or("") : "";
}

QDateTime ExternalView::getCreatedAt() const
{
    return mExternal ? mExternal->mCreatedAt.value_or(QDateTime{}) : QDateTime{};
}

QDateTime ExternalView::getUpdatedAt() const
{
    return mExternal ? mExternal->mUpdatedAt.value_or(QDateTime{}) : QDateTime{};
}

int ExternalView::getReadingTime() const
{
    return mExternal ? mExternal->mReadingTime.value_or(0) : 0;
}

ContentLabelList ExternalView::getContentLabels() const
{
    return mExternal ? ContentFilter::getContentLabels(mExternal->mLabels) : ContentLabelList{};
}

ExternalSource ExternalView::getSource() const
{
    return mExternal ? ExternalSource{mExternal->mSource} : ExternalSource{};
}

QVariant ExternalView::getAssociatedProfiles() const
{
    if (!mExternal)
        return {};

    BasicProfileList profileList;

    for (const auto& profile : mExternal->mAssociatedProfiles)
        profileList.push_back(BasicProfile{profile});

    return QVariant::fromValue(profileList);
}

}
