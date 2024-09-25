// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
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
    Q_INVOKABLE void transcodeVideo(const QString& inputFileName, int height, int startMs, int endMs);
    Q_INVOKABLE QString getVideoFileNameForGallery(const QString& extension);
    Q_INVOKABLE void copyVideoToGallery(const QString& fileName);

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
};

}
