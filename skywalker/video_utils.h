// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "video_cache.h"
#include "signal_object.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class VideoUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool transcoding READ isTranscoding NOTIFY transcodingChanged FINAL)
    QML_ELEMENT

public:
    explicit VideoUtils(QObject* parent = nullptr);

    bool isTranscoding() const { return mTranscoding; }
    void setTranscoding(bool transcoding);
    Q_INVOKABLE bool transcodeVideo(const QString& inputFileName, int height, int startMs, int endMs, bool removeAudio);
    Q_INVOKABLE QString getVideoFileNameForGallery(const QString& extension);
    Q_INVOKABLE void copyVideoToGallery(const QString& fileName);
    Q_INVOKABLE void indexGalleryFile(const QString& fileName);
    Q_INVOKABLE static void dropVideo(const QString& source);
    Q_INVOKABLE bool isTempVideoSource(const QString& source) const;
    Q_INVOKABLE bool videoSourceExists(const QString& source) const;

    Q_INVOKABLE VideoHandle* getVideoFromCache(const QString& link);
    Q_INVOKABLE VideoHandle* cacheVideo(const QString& link, const QString& fileName);

signals:
    void transcodingOk(QString inputFileName, QString outputFileName, int outputWidth, int outputHeight);
    void transcodingFailed(QString inputFileName, QString error);
    void transcodingChanged();
    void copyVideoOk();
    void copyVideoFailed(QString error);

private:
    void handleTranscodingOk(const QString& inputFileName, FileSignal::SharedPtr outputFile, int outputWidth, int outputHeight);
    void handleTranscodingFailed(const QString& inputFileName, FileSignal::SharedPtr outputFile, const QString& error);

    bool mTranscoding = false;
    QString mTranscodingFileName;
};

}
