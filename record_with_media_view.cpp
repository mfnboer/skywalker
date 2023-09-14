// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"
#include "record_with_media_view.h"

namespace Skywalker {

RecordWithMediaView::RecordWithMediaView(const ATProto::AppBskyEmbed::RecordWithMediaView* view) :
    mView(view)
{}

RecordView RecordWithMediaView::getRecord() const
{
    return mView ? RecordView(*mView->mRecord) : RecordView();
}

QList<ImageView> RecordWithMediaView::getImages() const
{
    if (!mView || mView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::Ptr>(mView->mMedia);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.append(ImageView(img.get()));

    return images;
}

QVariant RecordWithMediaView::getExternal() const
{
    if (!mView || mView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::Ptr>(mView->mMedia)->mExternal;
    return QVariant::fromValue(ExternalView(external.get()));
}

}
