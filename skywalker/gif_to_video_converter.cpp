// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "gif_to_video_converter.h"
#include "file_utils.h"
#include "temp_file_holder.h"

namespace Skywalker {

constexpr int VIDEO_BIT_RATE_SD = 4'000'000;
constexpr int VIDEO_BIT_RATE_HD = 8'000'000;

void GifToVideoConverter::convert(const QString& gifFileName)
{
    qDebug() << "Convert:" << gifFileName;
    mGif = std::make_unique<QMovie>(gifFileName);

    if (!mGif->isValid())
    {
        const QString error = mGif->lastErrorString();
        qWarning() << "Failed to open GIF:" << gifFileName << "error:" << error;
        emit conversionFailed(error);
        return;
    }

    qDebug() << "Format:" << mGif->format() << "supported:" << mGif->supportedFormats();
    mVideoFile = FileUtils::makeTempFile("mp4");

    if (!mVideoFile)
    {
        qWarning() << "Failed to create temp file";
        emit conversionFailed("Failed to create temp file");
        return;
    }

    mVideoFile->close();

    if (mGif->frameCount() < 3)
    {
        qWarning() << "Not enough frames:" << mGif->frameCount();
        emit conversionFailed("Not enough frames");
        return;
    }

    qDebug() << "Frame count:" << mGif->frameCount();
    const int fps = calcFps();

    if (fps < 0)
    {
        emit conversionFailed("Invalid duration");
        return;
    }

    mGif->jumpToFrame(0);
    const QImage firstFrame = mGif->currentImage();

    if (firstFrame.isNull())
    {
        qWarning() << "Cannot get first frame:" << gifFileName;
        emit conversionFailed("Cannot get first frame");
        return;
    }

    int width = firstFrame.width();

    // Resolution must be even for the video encoder
    if (width & 1)
        width += 1;

    int height = firstFrame.height();

    if (height & 1)
        height += 1;

    if (width != firstFrame.width() || height != firstFrame.height())
    {
        mGif->setScaledSize(QSize(width, height));
        qDebug() << "Scaled to:" << width << "x" << height;
    }

    int maxSize = std::max(width, height);
    const int bitRate = maxSize <= 720 ? VIDEO_BIT_RATE_SD : VIDEO_BIT_RATE_HD;
    mVideoEncoder = std::make_unique<VideoEncoder>();

    if (!mVideoEncoder->open(mVideoFile->fileName(), width, height, fps, bitRate))
    {
        qWarning() << "Cannot encode video";
        emit conversionFailed("Cannot encode video");
        return;
    }

    startThread();
}

void GifToVideoConverter::cancel()
{
    qDebug() << "Cancel";
    mCanceled = true;
}

void GifToVideoConverter::startThread()
{
    mCanceled = false;
    mThread = QThread::create([this]{ mConversionDone = pushFrames(); });

    if (!mThread)
    {
        qWarning() << "Failed to start thread";
        emit conversionFailed("Failed to start thread");
        return;
    }

    mThread->setParent(this);
    connect(mThread, &QThread::finished, this, [this]{ finished(); }, Qt::SingleShotConnection);
    mThread->start();
}

void GifToVideoConverter::finished()
{
    mVideoEncoder->close();
    mVideoEncoder = nullptr;
    mThread->deleteLater();
    mThread = nullptr;

    if (mCanceled)
        return;

    if (!mConversionDone)
    {
        emit conversionFailed("Conversion failed");
        return;
    }

    qDebug() << "Conversion finished";
    mVideoFile->flush();
    mVideoFile->close();
    const QString fileName = mVideoFile->fileName();
    TempFileHolder::instance().put(std::move(mVideoFile));
    emit conversionOk(fileName);
}

bool GifToVideoConverter::pushFrames()
{
    qDebug() << "Push frames to video encoder";
    mGif->jumpToFrame(0);
    const int frameCount = mGif->frameCount();
    int frameIndex = 0;

    do {
        qDebug() << "Push frame:" << frameIndex << "/" << frameCount << "next frame delay:" << mGif->nextFrameDelay();
        QImage frame = mGif->currentImage();
        const int frameDelayUs = mGif->nextFrameDelay() * 1000;

        if (frame.isNull())
        {
            qWarning() << "Failed to read frame:" << frameIndex;
            return false;
        }

        frame.convertTo(QImage::Format_RGBA8888); // must match format in QVideoEncoder.java

        if (!mVideoEncoder->push(frame, frameDelayUs))
        {
            qWarning() << "Failed to push frame to video enoder:" << frameIndex;
            return false;
        }

        emit conversionProgress(frameIndex / double(frameCount));

        if (mCanceled)
        {
            qDebug() << "Canceled";
            return false;
        }
    } while (mGif->jumpToNextFrame() && ++frameIndex <= frameCount);
    // Frames start counting at zero still there is a frame at index frameCount.
    // Seems frame 0 is not counted in the count??

    qDebug() << "All frames pushed";
    return true;
}

int GifToVideoConverter::calcFps()
{
    mGif->jumpToFrame(0);
    int totalMs = 0;
    int sampleCount = 0;

    // NOTE: fps is a hint to the video encoder. We sample 25 frames to get
    // an estimate of the average fps.
    while (mGif->jumpToNextFrame() && mGif->currentFrameNumber() <= 25)
    {
        totalMs += mGif->nextFrameDelay();
        ++sampleCount;
    }

    if (sampleCount <= 0)
    {
        qWarning() << "Invalid duration:" << totalMs;
        return 0;
    }

    const int avgFrameMs = totalMs / sampleCount;

    if (avgFrameMs <= 0)
    {
        qWarning() << "Invalid avg frame duration:" << avgFrameMs;
        return 0;
    }

    const int fps = std::max(1000 / avgFrameMs, 1);
    qDebug() << "fps:" << fps << "avgFrameMs:" << avgFrameMs << "samples:" << sampleCount << "tatalMs:" << totalMs;
    return fps;
}

}
