// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "image_view.h"
#include "cached_image_provider.h"

namespace Skywalker {

ImageView::ImageView(const QString& fullSizeUrl, const QString& alt, const QString& memeTopText, const QString& memeBottomText) :
    mFullSizeUrl(fullSizeUrl),
    mAlt(alt),
    mMemeTopText(memeTopText),
    mMemeBottomText(memeBottomText)
{
    if (mFullSizeUrl.startsWith("image://"))
        mFullSizeUrl = CachedImageProvider::createHttpUrl(mFullSizeUrl);
}

ImageView::ImageView(const QString& fullSizeUrl, const QString& alt, int width, int height) :
    mFullSizeUrl(fullSizeUrl),
    mAlt(alt),
    mWidth(width),
    mHeight(height)
{
    if (mFullSizeUrl.startsWith("image://"))
        mFullSizeUrl = CachedImageProvider::createHttpUrl(mFullSizeUrl);
}

ImageView::ImageView(const ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr& viewImage) :
    mViewImage(viewImage)
{}

ImageView::ImageView(const ATProto::AppBskyEmbed::GalleryViewImage::SharedPtr& viewImage) :
    mViewImage(viewImage)
{}

QString ImageView::getThumbUrl() const
{
    static const auto* provider = CachedImageProvider::getProvider(CachedImageProvider::THUMB_IMG);
    // Don't remove percent encoding like this: QUrl::fromPercentEncoding(getThumbHttpUrl().toUtf8())
    // It breaks a signature over the complete url if there is one.
    const QString thumbUrl = getThumbHttpUrl().toUtf8();
    return provider->createImageUrl(thumbUrl);
}

QString ImageView::getThumbHttpUrl() const
{
    if (ATProto::isNullVariant(mViewImage))
        return mFullSizeUrl;

    if (ATProto::holdsNonNull<ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr>(mViewImage))
        return std::get<ATProto::AppBskyEmbed::ImagesViewImage::SharedPtr>(mViewImage)->mThumb;

    if (ATProto::holdsNonNull<ATProto::AppBskyEmbed::GalleryViewImage::SharedPtr>(mViewImage))
        return std::get<ATProto::AppBskyEmbed::GalleryViewImage::SharedPtr>(mViewImage)->mThumbnail;

    qWarning() << "Unknown variant";
    return mFullSizeUrl;
}

QString ImageView::getFullSizeUrl() const
{
    if (ATProto::isNullVariant(mViewImage))
        return mFullSizeUrl;

    return std::visit([](auto&& viewImage){ return viewImage->mFullSize; }, mViewImage);
}

QString ImageView::getAlt() const
{
    if (!mHtmlAlt.isEmpty())
        return mHtmlAlt;

    if (ATProto::isNullVariant(mViewImage))
        return mAlt;

    return std::visit([](auto&& viewImage){ return viewImage->mAlt; }, mViewImage);
}

const ATProto::AppBskyEmbed::AspectRatio* ImageView::getAspectRatio() const
{
    if (ATProto::isNullVariant(mViewImage))
        return nullptr;

    return std::visit([](auto&& viewImage){ return viewImage->mAspectRatio.get(); }, mViewImage);
}

}
