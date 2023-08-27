// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"

namespace Skywalker
{

ExternalView::ExternalView(const ATProto::AppBskyEmbed::ExternalViewExternal* external) :
    mExternal(external)
{}

QString ExternalView::getUri() const
{
    return mExternal ? mExternal->mUri : QString();
}

QString ExternalView::getTitle() const
{
    return mExternal ? mExternal->mTitle : QString();
}

QString ExternalView::getDescription() const
{
    return mExternal ? mExternal->mDescription : QString();
}

QString ExternalView::getThumbUrl() const
{
    return (mExternal && mExternal->mThumb) ? *mExternal->mThumb : QString();
}

}
