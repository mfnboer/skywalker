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
    Q_PROPERTY(QList<ImageView> images READ getImages FINAL)
    Q_PROPERTY(QVariant external READ getExternal FINAL)
    QML_VALUE_TYPE(record_with_media_view)

public:
    using Ptr = std::unique_ptr<RecordWithMediaView>;

    RecordWithMediaView() = default;
    RecordWithMediaView(const ATProto::AppBskyEmbed::RecordWithMediaView* view);

    RecordView& getRecord() const;
    QList<ImageView> getImages() const;
    QVariant getExternal() const;

private:
    const ATProto::AppBskyEmbed::RecordWithMediaView* mView = nullptr;
    std::shared_ptr<RecordView> mRecordView;
};

}

Q_DECLARE_METATYPE(::Skywalker::RecordWithMediaView)
