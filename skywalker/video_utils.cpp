// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "video_utils.h"
#include "file_utils.h"
#include "jni_callback.h"
#include "temp_file_holder.h"

namespace Skywalker {

VideoUtils::VideoUtils(QObject* parent) :
    QObject(parent)
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingOk,
            this, [this](QString inputFileName, QString outputFileName){
                TempFileHolder::instance().put(outputFileName);
                emit transcodingOk(inputFileName, outputFileName);
            });

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingFailed,
            this, [this](QString inputFileName, QString outputFileName, QString error){
                QFile::remove(outputFileName);
                emit transcodingFailed(inputFileName, error);
            });
}

void VideoUtils::transcodeVideo(const QString inputFileName, int height, int startMs, int endMs)
{
    QFileInfo fileInfo(inputFileName);
    const QString ext = fileInfo.suffix();
    auto outputFile = FileUtils::makeTempFile(ext);
    const QString outputFileName = outputFile->fileName();
    outputFile = nullptr;

#if defined(Q_OS_ANDROID)
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject jsInputFileName = QJniObject::fromString(inputFileName);
    QJniObject jsOutputFileName = QJniObject::fromString(outputFileName);
    jint jiHeight = height;
    jint jiStartMs = startMs;
    jint jiEndMs = endMs;

    activity.callMethod<void>(
        "transcodeVideo",
        "(Ljava/lang/String;Ljava/lang/String;III)V",
        jsInputFileName.object<jstring>(),
        jsOutputFileName.object<jstring>(),
        jiHeight, jiStartMs, jiEndMs);
#else
    Q_UNUSED(height)
    Q_UNUSED(startMs)
    Q_UNUSED(endMs)
    qDebug() << "Transcoding not supported";
    QFile::copy(inputFileName, outputFileName);
    TempFileHolder::instance().put(outputFileName);
    emit transcodingOk(inputFileName, outputFileName);
#endif
}

}
