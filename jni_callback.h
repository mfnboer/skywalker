// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>

namespace Skywalker {

class JNICallbackListener : public QObject
{
    Q_OBJECT
public:
    static JNICallbackListener& getInstance();

    void handlePhotoPicked(const QString contentUri);
    void handlePhotoPickCanceled();

signals:
    void photoPicked(const QString contentUri);
    void photoPickCanceled();

private:
    JNICallbackListener();
};

}
