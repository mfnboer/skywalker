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
                handleTranscodingOk(inputFileName, outputFileName);
            });

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingFailed,
            this, [this](QString inputFileName, QString outputFileName, QString error){
                handleTranscodingFailed(inputFileName, outputFileName, error);
            });
}

void VideoUtils::setTranscoding(bool transcoding)
{
    if (transcoding != mTranscoding)
    {
        mTranscoding = transcoding;
        emit transcodingChanged();
    }
}

void VideoUtils::transcodeVideo(const QString& inputFileName, int height, int startMs, int endMs)
{
    auto outputFile = FileUtils::makeTempFile("mp4");
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

    setTranscoding(true);
#else
    Q_UNUSED(height)
    Q_UNUSED(startMs)
    Q_UNUSED(endMs)
    qDebug() << "Transcoding not supported";
    QFile::copy(inputFileName, outputFileName);
    handleTranscodingOk(inputFileName, outputFileName);
#endif
}

void VideoUtils::handleTranscodingOk(const QString& inputFileName, const QString& outputFileName)
{
    setTranscoding(false);
    TempFileHolder::instance().put(outputFileName);
    emit transcodingOk(inputFileName, outputFileName);
}

void VideoUtils::handleTranscodingFailed(const QString& inputFileName, const QString& outputFileName, const QString& error)
{
    setTranscoding(false);
    QFile::remove(outputFileName);
    emit transcodingFailed(inputFileName, error);
}

static QString createVideoFileName(QString extension)
{
    return QString("SKYWALKER_%1.%2").arg(FileUtils::createDateTimeName(), extension);
}

void VideoUtils::copyVideoToGallery(const QString& fileName)
{
    if (!FileUtils::checkWriteMediaPermission())
    {
        emit copyVideoFailed(tr("No permission to save video"));
        return;
    }

    const QString moviesPath = FileUtils::getMoviesPath();

    if (moviesPath.isEmpty())
    {
        emit copyVideoFailed(tr("No location to save video"));
        return;
    }

    const QFileInfo fileInfo(fileName);
    const QString outputFileName = QString("%1/%2").arg(moviesPath, createVideoFileName(fileInfo.suffix()));
    qDebug() << "Copy" << fileName << "to" << outputFileName;

    QFile inputFile(fileName);
    if (!inputFile.open(QFile::ReadOnly))
    {
        emit copyVideoFailed(tr("Failed read video"));
        return;
    }

    QFile outputFile(outputFileName);
    if (!outputFile.open(QFile::WriteOnly))
    {
        emit copyVideoFailed(tr("Failed write video"));
        return;
    }

    if (outputFile.write(inputFile.readAll()) < 0)
    {
        emit copyVideoFailed(tr("Failed to copy video to gallery"));
        return;
    }

    outputFile.flush();
    outputFile.close();
    FileUtils::scanMediaFile(outputFileName);
    emit copyVideoOk();
}

}
