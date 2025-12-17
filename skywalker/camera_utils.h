// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QQmlEngine>

namespace Skywalker {

class CameraUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CameraUtils(QObject* parent = nullptr);

    Q_INVOKABLE static bool checkCameraPermission();
    Q_INVOKABLE static QString getCaptureFileName();
};

}
