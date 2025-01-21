// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "video_utils.h"
#include "file_utils.h"
#include "jni_callback.h"
#include "post_utils.h"
#include "skywalker.h"
#include "temp_file_holder.h"

namespace Skywalker {

VideoUtils::VideoUtils(QObject* parent) :
    WrappedSkywalker(parent)
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

bool VideoUtils::transcodeVideo(const QString& inputFileName, int height, int startMs, int endMs, bool removeAudio)
{
    qDebug() << "Transcode video:" << inputFileName;

    if (mTranscoding)
    {
        qWarning() << "Transcoding still in progress";
        return false;
    }

    auto outputFile = FileUtils::makeTempFile("mp4");
    const QString outputFileName = outputFile->fileName();
    outputFile = nullptr;

#if defined(Q_OS_ANDROID)
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return false;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject jsInputFileName = QJniObject::fromString(inputFileName);
    QJniObject jsOutputFileName = QJniObject::fromString(outputFileName);
    jint jiHeight = height;
    jint jiStartMs = startMs;
    jint jiEndMs = endMs;
    jboolean jbRemoveAudio = removeAudio;

    activity.callMethod<void>(
        "transcodeVideo",
        "(Ljava/lang/String;Ljava/lang/String;IIIZ)V",
        jsInputFileName.object<jstring>(),
        jsOutputFileName.object<jstring>(),
        jiHeight, jiStartMs, jiEndMs, jbRemoveAudio);

    setTranscoding(true);
    mTranscodingFileName = inputFileName;
#else
    Q_UNUSED(height)
    Q_UNUSED(startMs)
    Q_UNUSED(endMs)
    Q_UNUSED(removeAudio)
    qDebug() << "Transcoding not supported";
    QFile::copy(inputFileName, outputFileName);
    handleTranscodingOk(inputFileName, outputFileName);
#endif
    return true;
}

void VideoUtils::handleTranscodingOk(const QString& inputFileName, const QString& outputFileName)
{
    if (inputFileName != mTranscodingFileName)
    {
        qDebug() << "Not for this instance:" << inputFileName << "transcoding:" << mTranscodingFileName;
        return;
    }

    qDebug() << "Transcoding ok:" << inputFileName;
    setTranscoding(false);
    mTranscodingFileName.clear();
    TempFileHolder::instance().put(outputFileName);
    emit transcodingOk(inputFileName, outputFileName);
}

void VideoUtils::handleTranscodingFailed(const QString& inputFileName, const QString& outputFileName, const QString& error)
{
    if (inputFileName != mTranscodingFileName)
    {
        qDebug() << "Not for this instance:" << inputFileName << "transcoding:" << mTranscodingFileName;
        return;
    }

    qWarning() << "Transcoding failed:" << inputFileName << error;
    setTranscoding(false);
    mTranscodingFileName.clear();
    QFile::remove(outputFileName);
    emit transcodingFailed(inputFileName, error);
}

static QString createVideoFileName(QString extension)
{
    return QString("SKYWALKER_%1.%2").arg(FileUtils::createDateTimeName(), extension);
}

QString VideoUtils::getVideoFileNameForGallery(const QString& extension)
{
    const QString moviesPath = FileUtils::getMoviesPath();

    if (moviesPath.isEmpty())
    {
        qWarning() << "No location to save video";
        return {};
    }

    const QString outputFileName = QString("%1/%2").arg(moviesPath, createVideoFileName(extension));
    return outputFileName;
}

void VideoUtils::copyVideoToGallery(const QString& fileName)
{
    if (!FileUtils::checkWriteMediaPermission())
    {
        emit copyVideoFailed(tr("No permission to save video"));
        return;
    }

    const QFileInfo fileInfo(fileName);
    const QString outputFileName = getVideoFileNameForGallery(fileInfo.suffix());

    if (outputFileName.isEmpty())
    {
        emit copyVideoFailed(tr("Cannot save to gallery"));
        return;
    }

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
    indexGalleryFile(outputFileName);
    emit copyVideoOk();
}

void VideoUtils::indexGalleryFile(const QString& fileName)
{
    qDebug() << "Index:" << fileName;
    FileUtils::scanMediaFile(fileName);
}

void VideoUtils::dropVideo(const QString& source)
{
    PostUtils::dropVideo(source);
}

void VideoUtils::setVideoTranscodedSource(const QString& postCid, const QString& source)
{
    if (postCid.isEmpty())
    {
        qDebug() << "No post cid for:" << source;
        return;
    }

    if (!mSkywalker)
        return;

    mSkywalker->makeLocalModelChange(
        [postCid, source](LocalPostModelChanges* model){
            model->updatePostVideoTranscodedSource(postCid, source);
        });
}

}
