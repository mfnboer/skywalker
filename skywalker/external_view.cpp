// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"

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
    if (!mTitle.isEmpty())
        return mTitle;

    return mExternal ? mExternal->mTitle : "";
}

QString ExternalView::getDescription() const
{
    if (!mDescription.isEmpty())
        return mDescription;

    return mExternal ? mExternal->mDescription : "";
}

QString ExternalView::getThumbUrl() const
{
    return mExternal ? mExternal->mThumb.value_or("") : "";
}

}
