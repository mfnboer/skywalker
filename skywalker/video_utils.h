// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class VideoUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit VideoUtils(QObject* parent = nullptr);

    Q_INVOKABLE void transcodeVideo(const QString inputFileName, int height, int startMs, int endMs);

signals:
    void transcodingOk(QString inputFileName, QString outputFileName);
    void transcodingFailed(QString inputFileName, QString error);
};

}
