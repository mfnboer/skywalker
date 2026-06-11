// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "external_view.h"
#include "record_with_media_view.h"
#include "video_view.h"

namespace Skywalker {

RecordWithMediaView::RecordWithMediaView(const ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr& view) :
    mView(view)
{}

RecordView& RecordWithMediaView::getRecord() const
{
    static RecordView NULL_RECORD_VIEW;

    if (mRecordView)
        return *mRecordView;

    if (!mView)
        return NULL_RECORD_VIEW;

    const_cast<RecordWithMediaView*>(this)->mRecordView = std::make_shared<RecordView>(*mView->mRecord);
    return *mRecordView;
}

RecordView::SharedPtr RecordWithMediaView::getRecordPtr() const
{
    if (mRecordView)
        return mRecordView;

    getRecord();

    return mRecordView;
}

void RecordWithMediaView::setRecord(const RecordView::SharedPtr& record)
{
    mRecordView = record;
}

bool RecordWithMediaView::hasUnknownEmbed() const
{
    return (mView && std::holds_alternative<ATProto::UnknownVariant::SharedPtr>(mView->mMedia));
}

QString RecordWithMediaView::getUnknownEmbedType() const
{
    if (!hasUnknownEmbed())
        return {};

    return std::get<ATProto::UnknownVariant::SharedPtr>(mView->mMedia)->mType;
}

QList<ImageView> RecordWithMediaView::getImages() const
{
    if (!mImages.empty())
        return mImages;

    if (!mView)
        return {};

    if (std::holds_alternative<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mView->mMedia))
    {
        const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mView->mMedia);
        QList<ImageView> images;

        for (const auto& img : imagesView->mImages)
            images.append(ImageView(img));

        return images;
    }
    else if (std::holds_alternative<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(mView->mMedia))
    {
        const auto& galleryView = std::get<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(mView->mMedia);
        QList<ImageView> images;

        for (const auto& item : galleryView->mItems)
        {
            const auto* image = std::get_if<ATProto::AppBskyEmbed::GalleryViewImage::SharedPtr>(&item);

            if (image)
                images.push_back(ImageView(*image));
        }

        return images;
    }

    return {};
}

bool RecordWithMediaView::hasImages() const
{
    if (!mImages.empty())
        return true;

    if (!mView)
        return false;

    return std::holds_alternative<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mView->mMedia) ||
           std::holds_alternative<ATProto::AppBskyEmbed::GalleryView::SharedPtr>(mView->mMedia);
}

QVariant RecordWithMediaView::getVideo() const
{
    auto videoView = getVideoView();

    if (!videoView)
        return {};

    return QVariant::fromValue(*videoView);
}

VideoView::Ptr RecordWithMediaView::getVideoView() const
{
    if (!mVideo.isNull())
        return std::make_unique<VideoView>(mVideo);

    if (!mView || !std::holds_alternative<ATProto::AppBskyEmbed::VideoView::SharedPtr>(mView->mMedia))
        return {};

    const auto& video = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(mView->mMedia);
    return std::make_unique<VideoView>(video);
}

bool RecordWithMediaView::hasVideo() const
{
    if (!mVideo.isNull())
        return true;

    if (!mView)
        return false;

    return std::holds_alternative<ATProto::AppBskyEmbed::VideoView::SharedPtr>(mView->mMedia);
}

QVariant RecordWithMediaView::getExternal() const
{
    auto externalView = getExternalView();

    if (!externalView)
        return {};

    return QVariant::fromValue(*externalView);
}

ExternalView::Ptr RecordWithMediaView::getExternalView() const
{
    if (!mExternal.isNull())
        return std::make_unique<ExternalView>(mExternal);

    if (!mView || !std::holds_alternative<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(mView->mMedia))
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(mView->mMedia)->mExternal;
    return std::make_unique<ExternalView>(external);
}

}
