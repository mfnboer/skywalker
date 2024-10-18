// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QtGlobal>
#include <QImage>

#if defined(Q_OS_ANDROID)
#include <QJniObject>
#endif

namespace Skywalker {

class VideoEncoder
{
public:
    bool open(const QString& fileName, int width, int height, int fps, int bitsPerFrame);
    bool close();
    bool push(const QImage& frame);

private:
#if defined(Q_OS_ANDROID)
    std::unique_ptr<QJniObject> mEncoder;
#endif
    int mWidth = 0;
    int mHeight = 0;
};

}
