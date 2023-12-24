// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "generator_view.h"
#include "user_settings.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

GeneratorViewerState::GeneratorViewerState(const ATProto::AppBskyFeed::GeneratorViewerState& viewerState) :
    mLike(viewerState.mLike.value_or(""))
{
}

GeneratorView::GeneratorView(const ATProto::AppBskyFeed::GeneratorView::SharedPtr& view) :
    mGeneratorView(view)
{
    Q_ASSERT(mGeneratorView);
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
    return mGeneratorView ? Profile(mGeneratorView->mCreator.get()) : Profile();
}

QString GeneratorView::getDisplayName() const
{
    return mGeneratorView ? mGeneratorView->mDisplayName : "";
}

QString GeneratorView::getDescription() const
{
    if (!mGeneratorView || !mGeneratorView->mDescription)
        return {};

    if (mGeneratorView->mDescriptionFacets.empty())
        return ATProto::PostMaster::plainToHtml(*mGeneratorView->mDescription);

    return ATProto::AppBskyRichtext::applyFacets(*mGeneratorView->mDescription,
                                                 mGeneratorView->mDescriptionFacets,
                                                 UserSettings::getLinkColor());
}

QString GeneratorView::getAvatar() const
{
    return mGeneratorView ? mGeneratorView->mAvatar.value_or("") : "";
}

int GeneratorView::getLikeCount() const
{
    return mGeneratorView ? mGeneratorView->mLikeCount : 0;
}

GeneratorViewerState GeneratorView::getViewer() const
{
    if (!mGeneratorView || !mGeneratorView->mViewer)
        return {};

    return GeneratorViewerState(*mGeneratorView->mViewer);
}


}
