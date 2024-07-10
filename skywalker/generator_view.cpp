// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "generator_view.h"
#include "content_filter.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

GeneratorViewerState::GeneratorViewerState(const ATProto::AppBskyFeed::GeneratorViewerState::SharedPtr& viewerState) :
    mViewerState(viewerState)
{
}

QString GeneratorViewerState::getLike() const
{
    return mViewerState ? mViewerState->mLike.value_or("") : "";
}

GeneratorView::GeneratorView(const ATProto::AppBskyFeed::GeneratorView::SharedPtr& view) :
    mGeneratorView(view)
{
}

QString GeneratorView::getUri() const
{
    return mGeneratorView ? mGeneratorView->mUri : "";
}

QString GeneratorView::getCid() const
{
    return mGeneratorView ? mGeneratorView->mCid : "";
}

QString GeneratorView::getDid() const
{
    return mGeneratorView ? mGeneratorView->mDid : "";
}

Profile GeneratorView::getCreator() const
{
    return mGeneratorView ? Profile(mGeneratorView->mCreator ) : Profile();
}

QString GeneratorView::getDisplayName() const
{
    return mGeneratorView ? mGeneratorView->mDisplayName : "";
}

QString GeneratorView::getDescription() const
{
    if (!mGeneratorView || !mGeneratorView->mDescription)
        return {};

    return *mGeneratorView->mDescription;
}

QString GeneratorView::getFormattedDescription() const
{
    if (!mGeneratorView || !mGeneratorView->mDescription)
        return {};

    return ATProto::RichTextMaster::getFormattedFeedDescription(*mGeneratorView, UserSettings::getLinkColor());
}

QString GeneratorView::getAvatar() const
{
    return mGeneratorView ? mGeneratorView->mAvatar.value_or("") : "";
}

ImageView GeneratorView::getImageView() const
{
    return ImageView(getAvatar(), getDisplayName());
}

int GeneratorView::getLikeCount() const
{
    return mGeneratorView ? mGeneratorView->mLikeCount : 0;
}

bool GeneratorView::acceptsInteractions() const
{
    return mGeneratorView ? mGeneratorView->mAcceptsInteractions : false;
}

ContentLabelList GeneratorView::getContentLabels() const
{
    return mGeneratorView ? ContentFilter::getContentLabels(mGeneratorView->mLabels) : ContentLabelList{};
}

GeneratorViewerState GeneratorView::getViewer() const
{
    if (!mGeneratorView || !mGeneratorView->mViewer)
        return {};

    return GeneratorViewerState(mGeneratorView->mViewer);
}

}
