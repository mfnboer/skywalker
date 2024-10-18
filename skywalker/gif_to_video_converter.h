// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "video_encoder.h"
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

signals:
    void conversionOk(QString fileName);
    void conversionFailed(QString error);

private:
    void startThread();
    void finished();
    bool pushFrames();

    VideoEncoder mVideoEncoder;
    std::unique_ptr<QMovie> mGif;
    std::unique_ptr<QTemporaryFile> mVideoFile;
    std::unique_ptr<QThread> mThread;
    bool mConversionDone = false;
};

}
