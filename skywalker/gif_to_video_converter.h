// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "video_encoder.h"
#include <QAtomicInt>
#include <QMovie>
#include <QObject>
#include <QTemporaryFile>
#include <QThread>
#include <QtQmlIntegration>

namespace Skywalker {

class GifToVideoConverter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_INVOKABLE void convert(const QString& gifFileName);
    Q_INVOKABLE void cancel();

signals:
    void conversionOk(QString fileName);
    void conversionFailed(QString error);
    void conversionProgress(double progress); // 0.0 => 1.0

private:
    void startThread();
    void finished();
    bool pushFrames();

    std::unique_ptr<VideoEncoder> mVideoEncoder;
    std::unique_ptr<QMovie> mGif;
    std::unique_ptr<QTemporaryFile> mVideoFile;
    std::unique_ptr<QThread> mThread;
    bool mConversionDone = false;
    QAtomicInteger<bool> mCanceled = false;
};

}
