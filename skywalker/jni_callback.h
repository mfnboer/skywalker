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

    void handlePhotoPicked(int fd, const QString mimeType);
    void handlePhotoPickCanceled();
    void handleSharedTextReceived(const QString sharedText);
    void handleSharedImageReceived(const QString fileName, const QString text);
    void handleSharedDmTextReceived(const QString sharedText);
    void handleShowNotifications();
    void handleShowDirectMessages();

signals:
    void photoPicked(int fd, QString mimeType);
    void photoPickCanceled();
    void sharedTextReceived(const QString sharedText);
    void sharedImageReceived(const QString fileName, const QString text);
    void sharedDmTextReceived(const QString sharedText);
    void showNotifications();
    void showDirectMessages();

private:
    JNICallbackListener();
};

}
