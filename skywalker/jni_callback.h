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

    // Checks if the app was launched due to an incoming intent. If so
    // the handling is started.
    static void handlePendingIntent();

    void handlePhotoPicked(int fd);
    void handlePhotoPickCanceled();
    void handleSharedTextReceived(const QString sharedText);
    void handleSharedImageReceived(const QString fileName, const QString text);
    void handlePause();
    void handleCheckNewMessages();

signals:
    void photoPicked(int fd);
    void photoPickCanceled();
    void sharedTextReceived(const QString sharedText);
    void sharedImageReceived(const QString fileName, const QString text);
    void appPause();
    void checkNewMessages();
private:
    JNICallbackListener();
};

}
