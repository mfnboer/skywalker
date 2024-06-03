// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"

namespace Skywalker
{

ExternalView::ExternalView(const ATProto::AppBskyEmbed::ExternalViewExternal* external) :
    mUri(external->mUri),
    mTitle(external->mTitle),
    mDescription(external->mDescription),
    mThumbUrl(external->mThumb.value_or(""))
{}

}
