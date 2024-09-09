// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_video.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class VideoUploadLimits
{
    Q_GADGET
    Q_PROPERTY(bool canUpload READ canUpload FINAL)
    Q_PROPERTY(int remainingDailyVideos READ getRemainingDailyVideos FINAL)
    Q_PROPERTY(int remainingDailyBytes READ getRemainingDailyBytes FINAL)
    Q_PROPERTY(QString error READ getError FINAL)
    Q_PROPERTY(QString message READ getMessage FINAL)
    QML_VALUE_TYPE(videouploadlimits)

public:
    VideoUploadLimits() = default;
    explicit VideoUploadLimits(const ATProto::AppBskyVideo::GetUploadLimitsOutput::SharedPtr& limits) : mLimits(limits) {}
    VideoUploadLimits(const QString& error, const QString& message) : mError(error), mMessage(message) {}

    bool canUpload() const { return mLimits ? mLimits->mCanUpload : false; }
    int getRemainingDailyVideos() const { return mLimits ? mLimits->mRemainingDailyVideos.value_or(0) : 0; }
    int getRemainingDailyBytes() const { return mLimits ? mLimits->mRemainingDailyBytes.value_or(0) : 0; }
    QString getError() const { return mLimits ? mLimits->mError.value_or("") : mError; }
    QString getMessage() const { return mLimits ? mLimits->mMessage.value_or("") : mMessage; }

private:
    ATProto::AppBskyVideo::GetUploadLimitsOutput::SharedPtr mLimits;
    QString mError;
    QString mMessage;
};

}
