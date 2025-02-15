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

QList<ImageView> RecordWithMediaView::getImages() const
{
    if (!mView || mView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::IMAGES_VIEW)
        return {};

    const auto& imagesView = std::get<ATProto::AppBskyEmbed::ImagesView::SharedPtr>(mView->mMedia);
    QList<ImageView> images;

    for (const auto& img : imagesView->mImages)
        images.append(ImageView(img));

    return images;
}

QVariant RecordWithMediaView::getVideo() const
{
    if (!mView || mView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::VIDEO_VIEW)
        return {};

    const auto& video = std::get<ATProto::AppBskyEmbed::VideoView::SharedPtr>(mView->mMedia);
    return  QVariant::fromValue(VideoView(video));
}

QVariant RecordWithMediaView::getExternal() const
{
    if (!mView || mView->mMediaType != ATProto::AppBskyEmbed::EmbedViewType::EXTERNAL_VIEW)
        return {};

    const auto& external = std::get<ATProto::AppBskyEmbed::ExternalView::SharedPtr>(mView->mMedia)->mExternal;
    return QVariant::fromValue(ExternalView(external));
}

}
