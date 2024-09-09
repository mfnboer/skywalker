// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "video_utils.h"
#include "file_utils.h"

namespace Skywalker {

VideoUtils::VideoUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
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
    return true;
#endif
}

}
