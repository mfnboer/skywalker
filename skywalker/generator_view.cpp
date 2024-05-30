// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "generator_view.h"
#include "content_filter.h"
#include "user_settings.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

GeneratorViewerState::GeneratorViewerState(const ATProto::AppBskyFeed::GeneratorViewerState& viewerState) :
    mLike(viewerState.mLike.value_or(""))
{
}

GeneratorView::GeneratorView(const ATProto::AppBskyFeed::GeneratorView::SharedPtr& view) :
    mSharedGeneratorView(view)
{
    Q_ASSERT(mSharedGeneratorView);
}

GeneratorView::GeneratorView(const ATProto::AppBskyFeed::GeneratorView* view) :
    mRawGeneratorView(view)
{
    Q_ASSERT(mRawGeneratorView);
}

QString GeneratorView::getUri() const
{
    return view() ? view()->mUri : "";
}

QString GeneratorView::getCid() const
{
    return view() ? view()->mCid : "";
}

QString GeneratorView::getDid() const
{
    return view() ? view()->mDid : "";
}

Profile GeneratorView::getCreator() const
{
    return view() ? Profile(view()->mCreator.get()) : Profile();
}

QString GeneratorView::getDisplayName() const
{
    return view() ? view()->mDisplayName : "";
}

QString GeneratorView::getDescription() const
{
    if (!view() || !view()->mDescription)
        return {};

    return *view()->mDescription;
}

QString GeneratorView::getFormattedDescription() const
{
    if (!view() || !view()->mDescription)
        return {};

    return ATProto::RichTextMaster::getFormattedFeedDescription(*view(), UserSettings::getLinkColor());
}

QString GeneratorView::getAvatar() const
{
    return view() ? view()->mAvatar.value_or("") : "";
}

ImageView GeneratorView::getImageView() const
{
    return ImageView(getAvatar(), getDisplayName());
}

int GeneratorView::getLikeCount() const
{
    return view() ? view()->mLikeCount : 0;
}

bool GeneratorView::acceptsInteractions() const
{
    return view() ? view()->mAcceptsInteractions : false;
}

ContentLabelList GeneratorView::getContentLabels() const
{
    return view() ? ContentFilter::getContentLabels(view()->mLabels) : ContentLabelList{};
}

GeneratorViewerState GeneratorView::getViewer() const
{
    if (!view() || !view()->mViewer)
        return {};

    return GeneratorViewerState(*view()->mViewer);
}

const ATProto::AppBskyFeed::GeneratorView* GeneratorView::view() const
{
    return mSharedGeneratorView ? mSharedGeneratorView.get() : mRawGeneratorView;
}

}
