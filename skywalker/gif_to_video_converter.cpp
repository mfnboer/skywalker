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

    // Frames start at 0, but the delay between frame 0 and 1 seems not always right
    mGif->jumpToFrame(0);
    mGif->jumpToNextFrame();

    const QImage firstFrame = mGif->currentImage();

    if (firstFrame.isNull())
    {
        qWarning() << "Cannot get first frame:" << gifFileName;
        emit conversionFailed("Cannot get first frame");
        return;
    }

    const int frameDelayMs = mGif->nextFrameDelay();
    qDebug() << "Frame delay ms:" << frameDelayMs;

    if (frameDelayMs <= 0)
    {
        qWarning() << "Invalid frame delay:" << frameDelayMs;
        emit conversionFailed("Invalid frame delay");
        return;
    }

    const int fps = std::max(1000 / frameDelayMs, 1);
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

void GifToVideoConverter::startThread()
{
    QThread* thread = QThread::create([this]{ mConversionDone = pushFrames(); });

    if (!thread)
    {
        qWarning() << "Failed to start thread";
        emit conversionFailed("Failed to start thread");
        return;
    }

    mThread.reset(thread);
    connect(thread, &QThread::finished, this, [this]{ finished(); }, Qt::SingleShotConnection);
    mThread->start();
}

void GifToVideoConverter::finished()
{
    mVideoEncoder->close();
    mVideoEncoder = nullptr;

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
        const QImage frame = mGif->currentImage();

        if (frame.isNull())
        {
            qWarning() << "Failed to read frame:" << frameIndex;
            return false;
        }

        if (!mVideoEncoder->push(frame))
        {
            qWarning() << "Failed to push frame to video enoder:" << frameIndex;
            return false;
        }
    } while (mGif->jumpToNextFrame() && ++frameIndex <= frameCount);
    // Frames start counting at zero still there is a frame at index frameCount.
    // Seems frame 0 is not counted in the count??

    qDebug() << "All frames pushed";
    return true;
}

}
