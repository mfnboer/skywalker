// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "video_encoder.h"
#include <QDebug>

namespace Skywalker {

bool VideoEncoder::open(const QString& fileName, int width, int height, int fps, int bitRate)
{
    qDebug() << "file:" << fileName << "width:" << width << "height:" << height << "fps:" << fps << "bitRate:" << bitRate;
#if defined(Q_OS_ANDROID)
    Q_ASSERT(!mEncoder);
    // Dimension must be even numbers for the H.264 encoder
    Q_ASSERT((width & 1) == 0);
    Q_ASSERT((height & 1) == 0);
    mWidth = width;
    mHeight = height;
    auto jsFile = QJniObject::fromString(fileName);
    mEncoder = std::make_unique<QJniObject>("com/gmail/mfnboer/QVideoEncoder");
    const jboolean result = mEncoder->callMethod<jboolean>("init", "(Ljava/lang/String;IIII)Z",
                                                           jsFile.object<jstring>(),
                                                           (jint)width, (jint)height, (jint)fps,
                                                           (jint)bitRate);
    return (bool)result;
#else
    Q_UNUSED(fileName);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(fps);
    Q_UNUSED(bitRate);
    qWarning() << "Video encoding not supported!";
    return false;
#endif
}

bool VideoEncoder::close()
{
#if defined(Q_OS_ANDROID)
    if (mEncoder)
    {
        mEncoder->callMethod<void>("release", "()V");
        mEncoder = nullptr;
    }

    return true;
#else
    qWarning() << "Video encoding not supported!";
    return false;
#endif
}

bool VideoEncoder::push(const QImage& frame)
{
#if defined(Q_OS_ANDROID)
    Q_ASSERT(mWidth == frame.width());
    Q_ASSERT(mHeight == frame.height());
    QJniEnvironment env;
    const int size = frame.width() * frame.height() * 4;
    auto jsFrame = env->NewByteArray(size);
    const uint8_t* frameBits = frame.constBits();
    env->SetByteArrayRegion(jsFrame, 0, size, (jbyte*)frameBits);
    auto added = mEncoder->callMethod<jboolean>("addFrame", "([B)Z", jsFrame);
    env->DeleteLocalRef(jsFrame);
    return (bool)added;
#else
    Q_UNUSED(frame);
    qWarning() << "Video encoding not supported!";
    return false;
#endif
}

}
