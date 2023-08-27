// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker
{

class ImageView
{
    Q_GADGET
    Q_PROPERTY(QString thumbUrl READ getThumbUrl FINAL)
    Q_PROPERTY(QString fullSizeUrl READ getFullSizeUrl FINAL)
    Q_PROPERTY(QString alt READ getAlt FINAL)
    QML_VALUE_TYPE(imageview)

public:
    using Ptr = std::unique_ptr<ImageView>;

    ImageView() = default;
    ImageView(const ATProto::AppBskyEmbed::ImagesViewImage* viewImage) :
        mViewImage(viewImage)
    {}

    QString getThumbUrl() const { return mViewImage ? mViewImage->mThumb : QString(); }
    QString getFullSizeUrl() const { return mViewImage ? mViewImage->mFullSize : QString(); }
    QString getAlt() const { return mViewImage ? mViewImage->mAlt : QString(); }

private:
    const ATProto::AppBskyEmbed::ImagesViewImage* mViewImage = nullptr;
};

}

Q_DECLARE_METATYPE(Skywalker::ImageView)
