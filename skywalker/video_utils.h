// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "wrapped_skywalker.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class VideoUtils : public WrappedSkywalker
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
    Q_INVOKABLE void setVideoTranscodedSource(const QString& postCid, const QString& source);
    Q_INVOKABLE bool isTempVideoSource(const QString& source) const;

signals:
    void transcodingOk(QString inputFileName, QString outputFileName);
    void transcodingFailed(QString inputFileName, QString error);
    void transcodingChanged();
    void copyVideoOk();
    void copyVideoFailed(QString error);

private:
    void handleTranscodingOk(const QString& inputFileName, const QString& outputFileName);
    void handleTranscodingFailed(const QString& inputFileName, const QString& outputFileName, const QString& error);

    bool mTranscoding = false;
    QString mTranscodingFileName;
};

}
