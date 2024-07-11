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
    Q_PROPERTY(int width READ getWidth FINAL)
    Q_PROPERTY(int height READ getHeight FINAL)
    Q_PROPERTY(QString memeTopText READ getMemeTopText FINAL)
    Q_PROPERTY(QString memeBottomText READ getMemeBottomText FINAL)
    QML_VALUE_TYPE(imageview)

public:
    using Ptr = std::unique_ptr<ImageView>;

    ImageView() = default;
    ImageView(const QString& fullSizeUrl, const QString& alt, const QString& memeTopText = "", const QString& memeBottomText = "") :
        mFullSizeUrl(fullSizeUrl),
        mAlt(alt),
        mMemeTopText(memeTopText),
        mMemeBottomText(memeBottomText)
    {}
    ImageView(const ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr& viewImage) :
        mViewImage(viewImage)
    {}

    QString getThumbUrl() const { return mViewImage ? mViewImage->mThumb : mFullSizeUrl; }
    QString getFullSizeUrl() const { return mViewImage ? mViewImage->mFullSize : mFullSizeUrl; }
    QString getAlt() const { return mViewImage ? mViewImage->mAlt : mAlt; }
    const ATProto::AppBskyEmbed::AspectRatio* getAspectRatio() const { return mViewImage ? mViewImage->mAspectRatio.get() : nullptr; }
    int getWidth() const { auto* r = getAspectRatio(); return r ? r->mWidth : 0;  }
    int getHeight() const { auto* r = getAspectRatio(); return r ? r->mHeight : 0;  }
    const QString& getMemeTopText() const { return mMemeTopText; }
    void setMemeTopText(const QString& text) { mMemeTopText = text; }
    const QString& getMemeBottomText() const { return mMemeBottomText; }
    void setMemeBottomText(const QString& text) { mMemeBottomText = text; }

private:
    ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr mViewImage;

    QString mFullSizeUrl;
    QString mAlt;

    // Only for draft posts
    QString mMemeTopText;
    QString mMemeBottomText;
};

}

Q_DECLARE_METATYPE(::Skywalker::ImageView)
