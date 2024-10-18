// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "gif_to_video_converter.h"
#include "file_utils.h"
#include "temp_file_holder.h"

namespace Skywalker {

constexpr int VIDEO_BIT_RATE = 8'000'000;

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

    mVideoFile = FileUtils::makeTempFile("mp4");

    if (!mVideoFile)
    {
        qWarning() << "Failed to create temp file";
        emit conversionFailed("Failed to create temp file");
        return;
    }

    mVideoFile->close();

    if (mGif->frameCount() < 2)
    {
        qWarning() << "Not enough frames:" << mGif->frameCount();
        emit conversionFailed("Not enough frames");
        return;
    }

    qDebug() << "Frame count:" << mGif->frameCount();
    mGif->jumpToFrame(0);
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
    const int bitsPerFrame = VIDEO_BIT_RATE / fps;

    if (!mVideoEncoder.open(mVideoFile->fileName(), firstFrame.width(), firstFrame.height(), fps, bitsPerFrame))
    {
        qWarning() << "Cannot encode video";
        emit conversionFailed("Cannot encode video");
        return;
    }
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
    if (!mConversionDone)
    {
        emit conversionFailed("Conversion failed");
        return;
    }

    qDebug() << "Conversion finished";
    const QString fileName = mVideoFile->fileName();
    TempFileHolder::instance().put(std::move(mVideoFile));
    emit conversionOk(fileName);
}

bool GifToVideoConverter::pushFrames()
{
    qDebug() << "Push frames to video encoder";

    do {
        const QImage frame = mGif->currentImage();

        if (frame.isNull())
        {
            qWarning() << "Failed to read frame";
            return false;
        }

        if (!mVideoEncoder.push(frame))
        {
            qWarning() << "Failed to push frame to video enoder";
            return false;
        }
    } while (mGif->jumpToNextFrame());

    qDebug() << "All frames pushed";
    return true;
}

}
