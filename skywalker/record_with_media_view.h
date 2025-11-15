// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_view.h"
#include "record_view.h"

namespace Skywalker {

class RecordWithMediaView
{
    Q_GADGET
    Q_PROPERTY(RecordView record READ getRecord FINAL)
    Q_PROPERTY(bool hasUnknownEmbed READ hasUnknownEmbed FINAL)
    Q_PROPERTY(QString unknownEmbedType READ getUnknownEmbedType FINAL)
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
    Q_PROPERTY(QVariant video READ getVideo FINAL)
    Q_PROPERTY(QVariant external READ getExternal FINAL)
    QML_VALUE_TYPE(record_with_media_view)

public:
    using Ptr = std::unique_ptr<RecordWithMediaView>;

    RecordWithMediaView() = default;
    RecordWithMediaView(const ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr& view);

    RecordView& getRecord() const;
    RecordView::SharedPtr getRecordPtr() const;
    void setRecord(const RecordView::SharedPtr& record);
    bool hasUnknownEmbed() const;
    QString getUnknownEmbedType() const;
    QList<ImageView> getImages() const;
    bool hasImages() const;
    QVariant getVideo() const;
    VideoView::Ptr getVideoView() const;
    bool hasVideo() const;
    QVariant getExternal() const;
    ExternalView::Ptr getExternalView() const;

    void setImages(const QList<ImageView>& images) { mImages = images; }
    void setVideo(const VideoView& video) { mVideo = video; }
    void setExternal(const ExternalView& external) { mExternal = external; }

private:
    ATProto::AppBskyEmbed::RecordWithMediaView::SharedPtr mView;
    RecordView::SharedPtr mRecordView;
    QList<ImageView> mImages;
    VideoView mVideo;
    ExternalView mExternal;
};

}

Q_DECLARE_METATYPE(::Skywalker::RecordWithMediaView)
