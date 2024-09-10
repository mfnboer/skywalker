// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "video_utils.h"
#include "file_utils.h"
#include "jni_callback.h"

namespace Skywalker {

VideoUtils::VideoUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    auto& jniCallbackListener = JNICallbackListener::getInstance();

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingOk,
            this, [this](QString inputFileName, QString outputFileName){
                emit transcodingOk(inputFileName, outputFileName);
            });

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingFailed,
            this, [this](QString inputFileName, QString outputFileName, QString error){
                emit transcodingFailed(inputFileName, outputFileName, error);
            });
}

void VideoUtils::transcodeVideo(const QString inputFileName)
{
#if defined(Q_OS_ANDROID)
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QFileInfo fileInfo(inputFileName);
    const QString ext = fileInfo.suffix();
    auto outputFile = FileUtils::makeTempFile(ext);
    const QString outputFileName = outputFile->fileName();
    outputFile = nullptr;

    QJniObject jsInputFileName = QJniObject::fromString(inputFileName);
    QJniObject jsOutputFileName = QJniObject::fromString(outputFileName);
    jint jHeight = 720;

    activity.callMethod<void>(
        "transcodeVideo",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        jsInputFileName.object<jstring>(),
        jsOutputFileName.object<jstring>(),
        jHeight);
#else
    // TODO
    Q_UNUSED(inputFileName)
#endif
}

}
