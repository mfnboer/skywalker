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

    void handlePhotoPicked(int fd, const QString& mimeType);
    void handlePhotoPickCanceled();
    void handleVideoTranscodingOk(const QString& inputFileName, const QString& outputFileName);
    void handleVideoTranscodingFailed(const QString& inputFileName, const QString& outputFileName, const QString& error);
    void handleSharedTextReceived(const QString& sharedText);
    void handleSharedImageReceived(const QString& contentUri, const QString& text);
    void handleSharedVideoReceived(const QString& contentUri, const QString& text);
    void handleSharedDmTextReceived(const QString& sharedText);
    void handleShowNotifications();
    void handleShowDirectMessages();

signals:
    void photoPicked(int fd, QString mimeType);
    void photoPickCanceled();
    void videoTranscodingOk(QString inputFileName, QString outputFileName);
    void videoTranscodingFailed(QString inputFileName, QString outputFileName, QString error);
    void sharedTextReceived(QString sharedText);
    void sharedImageReceived(QString contentUri, QString text);
    void sharedVideoReceived(QString contentUri, QString text);
    void sharedDmTextReceived(QString sharedText);
    void showNotifications();
    void showDirectMessages();

private:
    JNICallbackListener();
};

}
