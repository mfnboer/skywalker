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
    Q_PROPERTY(int startMs READ getStartMs FINAL)
    Q_PROPERTY(int endMs READ getEndMs FINAL)
    Q_PROPERTY(int newHeight READ getNewHeight FINAL)
    QML_VALUE_TYPE(videoview)

public:
    using Ptr = std::unique_ptr<VideoView>;

    VideoView() = default;
    VideoView(const ATProto::AppBskyEmbed::VideoView::SharedPtr& videoView) : mVideoView(videoView) {}
    VideoView(const QString& playListUrl, const QString& alt, int startMs, int endMs, int newHeight) :
        mPlayListUrl(playListUrl), mAlt(alt), mStartMs(startMs), mEndMs(endMs), mNewHeight(newHeight) {}

    Q_INVOKABLE bool isNull() const { return getPlaylistUrl().isEmpty(); }
    QString getThumbUrl() const { return mVideoView && mVideoView->mThumbnail ? *mVideoView->mThumbnail : ""; }
    QString getPlaylistUrl() const { return mVideoView ? mVideoView->mPlaylist : mPlayListUrl; }
    //QString getPlaylistUrl() const { return "https://video.bsky.app/watch/did%3Aplc%3Aoisofpd7lj26yvgiivf3lxsi/bafkreihc3xae3wohhhgveo3ydjaee2dlrpfe726pz7nntyg7tiqd55o6pu/360p/video.m3u8?session_id=crgs65kcgcls72stgg1g"; } // TODO TEST
    //QString getPlaylistUrl() const { return "https://video.bsky.app/watch/did%3Aplc%3Aqrllvid7s54k4hnwtqxwetrf/bafkreifaocag4dsbz3kfm446hje5zyw3sohcpmozjyrnebseukcqgc5iwa/360p/video0.ts?session_id=crgu8crij0ss72uv4v5g&dur=8.333333"; } // TODO TEST
    const ATProto::AppBskyEmbed::AspectRatio* getAspectRatio() const { return mVideoView ? mVideoView->mAspectRatio.get() : nullptr; }
    int getWidth() const { auto* r = getAspectRatio(); return r ? r->mWidth : 0;  }
    int getHeight() const { auto* r = getAspectRatio(); return r ? r->mHeight : 0;  }
    QString getAlt() const { return mVideoView && mVideoView->mAlt ? *mVideoView->mAlt : mAlt; }
    ImageView getImageView() const { return ImageView(getThumbUrl(), getAlt()); }
    int getStartMs() const { return mStartMs; }
    void setStartMs(int startMs) { mStartMs = startMs; }
    int getEndMs() const { return mEndMs; }
    void setEndMs(int endMs) { mEndMs = endMs; }
    int getNewHeight() const { return mNewHeight; }
    void setNewHeight(int newHeight) { mNewHeight = newHeight; }

    const ATProto::AppBskyEmbed::VideoView::SharedPtr& getATProtoView() const { return mVideoView; }

private:
    ATProto::AppBskyEmbed::VideoView::SharedPtr mVideoView;
    QString mPlayListUrl;
    QString mAlt;

    // Only for draft posts
    int mStartMs = 0;
    int mEndMs = 0;
    int mNewHeight = 0;
};

}
