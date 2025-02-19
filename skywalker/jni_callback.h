// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "signal_object.h"
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
    void handleExtractTextAvailabilityOk(QEnums::Script script, bool available);
    void handleExtractTextAvailabilityFailed(QEnums::Script script, const QString& error);
    void handleExtractTextInstallProgress(QEnums::Script script, int progressPercentage);
    void handleExtractTextInstallOk(QEnums::Script script);
    void handleExtractTextInstallFailed(QEnums::Script script, const QString& error);
    void handleExtractTextOk(const QString& imgSource, const QString& text);
    void handleExtractTextFailed(const QString& imgSource, const QString& error);
    void handleLanguageIdentified(const QString& languageCode, int requestId);
    void handleSharedTextReceived(const QString& sharedText);
    void handleSharedImageReceived(const QString& contentUri, const QString& text);
    void handleSharedVideoReceived(const QString& contentUri, const QString& text);
    void handleSharedDmTextReceived(const QString& sharedText);
    void handleShowNotifications();
    void handleShowDirectMessages();
    void handleKeyboardHeightChanged(int height);

signals:
    void photoPicked(int fd, QString mimeType);
    void photoPickCanceled();
    void videoTranscodingOk(QString inputFileName, FileSignal::SharedPtr outputFile);
    void videoTranscodingFailed(QString inputFileName, FileSignal::SharedPtr outputFile, QString error);
    void extractTextAvailabilityOk(QEnums::Script script, bool available);
    void extractTextAvailabilityFailed(QEnums::Script script, QString error);
    void extractTextInstallProgress(QEnums::Script script, int progressPercentage);
    void extractTextInstallOk(QEnums::Script script);
    void extractTextInstallFailed(QEnums::Script script, QString error);
    void extractTextOk(QString imgSource, QString text);
    void extractTextFailed(QString imgSource, QString error);
    void languageIdentified(QString languageCode, int requestId);
    void sharedTextReceived(QString sharedText);
    void sharedImageReceived(QString contentUri, QString text);
    void sharedVideoReceived(QString contentUri, QString text);
    void sharedDmTextReceived(QString sharedText);
    void showNotifications();
    void showDirectMessages();
    void keyboardHeightChanged(int height);

private:
    JNICallbackListener();
};

}
