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
            this, [this](QString inputFileName, FileSignal::SharedPtr outputFile){
                handleTranscodingOk(inputFileName, outputFile);
            });

    connect(&jniCallbackListener, &JNICallbackListener::videoTranscodingFailed,
            this, [this](QString inputFileName, FileSignal::SharedPtr outputFile, QString error){
                handleTranscodingFailed(inputFileName, outputFile, error);
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

    auto outputFile = FileUtils::makeTempFile("mp4", true);
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
    handleTranscodingOk(inputFileName, std::make_shared<FileSignal>(outputFileName));
#endif
    return true;
}

void VideoUtils::handleTranscodingOk(const QString& inputFileName, FileSignal::SharedPtr outputFile)
{
    if (inputFileName != mTranscodingFileName)
    {
        qDebug() << "Not for this instance:" << inputFileName << "transcoding:" << mTranscodingFileName;
        return;
    }

    qDebug() << "Transcoding ok:" << inputFileName;
    setTranscoding(false);
    mTranscodingFileName.clear();
    outputFile->setHandled(true);
    TempFileHolder::instance().put(outputFile->getFileName());
    emit transcodingOk(inputFileName, outputFile->getFileName());
}

void VideoUtils::handleTranscodingFailed(const QString& inputFileName, FileSignal::SharedPtr outputFile, const QString& error)
{
    if (inputFileName != mTranscodingFileName)
    {
        qDebug() << "Not for this instance:" << inputFileName << "transcoding:" << mTranscodingFileName;
        return;
    }

    qWarning() << "Transcoding failed:" << inputFileName << error;
    setTranscoding(false);
    mTranscodingFileName.clear();
    outputFile->setHandled(true);
    QFile::remove(outputFile->getFileName());
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

bool VideoUtils::isTempVideoSource(const QString& source) const
{
    if (!source.startsWith("file://"))
        return false;

    QFileInfo info(source.sliced(7));
    return info.suffix() == "mp4" && info.baseName().startsWith(TempFileHolder::namePrefix());
}

VideoHandle* VideoUtils::getVideoFromCache(const QString& link)
{
    auto* handle = VideoCache::instance().getVideo(link);
    handle->setParent(this);
    return handle;
}

VideoHandle* VideoUtils::cacheVideo(const QString& link, const QString& fileName)
{
    auto* handle = VideoCache::instance().putVideo(link, fileName);
    handle->setParent(this);
    return handle;
}

}
