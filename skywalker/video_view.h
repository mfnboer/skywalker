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
    Q_PROPERTY(bool removeAudio READ getRemoveAudio FINAL)
    Q_PROPERTY(int newHeight READ getNewHeight FINAL)
    QML_VALUE_TYPE(videoview)

public:
    using Ptr = std::unique_ptr<VideoView>;

    VideoView() = default;
    VideoView(const ATProto::AppBskyEmbed::VideoView::SharedPtr& videoView) : mVideoView(videoView) {}
    VideoView(const QString& playListUrl, const QString& alt, int startMs, int endMs, bool removeAudio, int newHeight) :
        mPlayListUrl(playListUrl), mAlt(alt), mStartMs(startMs), mEndMs(endMs), mRemoveAudio(removeAudio), mNewHeight(newHeight) {}

    Q_INVOKABLE bool isNull() const { return getPlaylistUrl().isEmpty(); }
    QString getThumbUrl() const { return mVideoView && mVideoView->mThumbnail ? *mVideoView->mThumbnail : ""; }
    QString getPlaylistUrl() const { return mVideoView ? mVideoView->mPlaylist : mPlayListUrl; }
    const ATProto::AppBskyEmbed::AspectRatio* getAspectRatio() const { return mVideoView ? mVideoView->mAspectRatio.get() : nullptr; }
    int getWidth() const { auto* r = getAspectRatio(); return r ? r->mWidth : 0;  }
    int getHeight() const { auto* r = getAspectRatio(); return r ? r->mHeight : 0;  }
    QString getAlt() const { return !mHtmlAlt.isEmpty() ? mHtmlAlt : (mVideoView && mVideoView->mAlt ? *mVideoView->mAlt : mAlt); }

    ImageView getImageView() const {
        ImageView imageView(getThumbUrl(), getAlt(), getWidth(), getHeight());

        if (hasHtmlAlt())
            imageView.setHtmlAlt(mHtmlAlt);

        return imageView;
    }

    int getStartMs() const { return mStartMs; }
    void setStartMs(int startMs) { mStartMs = startMs; }
    int getEndMs() const { return mEndMs; }
    void setEndMs(int endMs) { mEndMs = endMs; }
    bool getRemoveAudio() const { return mRemoveAudio; }
    void setRemoveAudio(bool removeAudio) { mRemoveAudio = removeAudio; }
    int getNewHeight() const { return mNewHeight; }
    void setNewHeight(int newHeight) { mNewHeight = newHeight; }

    Q_INVOKABLE bool hasHtmlAlt() const { return !mHtmlAlt.isEmpty(); }
    void setHtmlAlt(const QString& htmlAlt) { mHtmlAlt = htmlAlt; }

    const ATProto::AppBskyEmbed::VideoView::SharedPtr& getATProtoView() const { return mVideoView; }

private:
    ATProto::AppBskyEmbed::VideoView::SharedPtr mVideoView;
    QString mPlayListUrl;
    QString mAlt;
    QString mHtmlAlt;

    // Only for draft posts
    int mStartMs = 0;
    int mEndMs = 0;
    bool mRemoveAudio = false;
    int mNewHeight = 0;
};

}
