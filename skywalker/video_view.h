// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "image_view.h"
#include <atproto/lib/lexicon/app_bsky_embed.h>
#include <QtQmlIntegration>

namespace Skywalker
{

class VideoView
{
    Q_GADGET
    Q_PROPERTY(QString thumbUrl READ getThumbUrl FINAL)
    Q_PROPERTY(QString playlistUrl READ getPlaylistUrl FINAL)
    Q_PROPERTY(QString alt READ getAlt FINAL)
    Q_PROPERTY(int width READ getWidth FINAL)
    Q_PROPERTY(int height READ getHeight FINAL)
    Q_PROPERTY(ImageView imageView READ getImageView FINAL)
    QML_VALUE_TYPE(videoview)

public:
    using Ptr = std::unique_ptr<VideoView>;

    VideoView() = default;
    VideoView(const ATProto::AppBskyEmbed::VideoView::SharedPtr& videoView) : mVideoView(videoView) {}

    QString getThumbUrl() const { return mVideoView && mVideoView->mThumbnail ? *mVideoView->mThumbnail : ""; }
    QString getPlaylistUrl() const { return mVideoView ? mVideoView->mPlaylist : ""; }
    const ATProto::AppBskyEmbed::AspectRatio* getAspectRatio() const { return mVideoView ? mVideoView->mAspectRatio.get() : nullptr; }
    int getWidth() const { auto* r = getAspectRatio(); return r ? r->mWidth : 0;  }
    int getHeight() const { auto* r = getAspectRatio(); return r ? r->mHeight : 0;  }
    QString getAlt() const { return mVideoView && mVideoView->mAlt ? *mVideoView->mAlt : ""; }
    ImageView getImageView() const { return ImageView(getThumbUrl(), getAlt()); }

private:
    ATProto::AppBskyEmbed::VideoView::SharedPtr mVideoView;
};

}
