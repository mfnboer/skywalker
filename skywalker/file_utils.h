// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>
#include <QDateTime>
#include <QObject>
#include <QTemporaryFile>
#include <QQmlEngine>

namespace Skywalker {

class FileUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum class FileMode
    {
        READ_ONLY,
        WRITE_ONLY
    };

    static bool checkReadMediaPermission();
    static bool checkWriteMediaPermission();
    static bool isPhotoPickerAvailable();
    static QString getAppDataPath(const QString& subDir);
    static QString getPicturesPath();
    static QString getPicturesPath(const QString& subDir);
    static QString getMoviesPath();
    static QString getCachePath(const QString& subDir);
    static int openContentUri(const QString& contentUri, FileMode mode);
    static Q_INVOKABLE bool deleteContentUri(const QString& contentUri);
    static std::unique_ptr<QFile> openFile(const QUrl& fileUri, FileMode mode);
    static Q_INVOKABLE QString resolveContentUriToFileName(const QString& contentUri);
    static std::unique_ptr<QTemporaryFile> makeTempFile(const QString& fileExtension, bool cache = false);
    static std::unique_ptr<QTemporaryFile> createTempFile(const QString& fileUri, const QString& fileExtension);
    static std::unique_ptr<QTemporaryFile> createTempFile(QFile& file, const QString& fileExtension);
    static std::unique_ptr<QTemporaryFile> createTempFile(int fd, const QString& fileExtension);
    static QString createDateTimeName(QDateTime timestamp = QDateTime::currentDateTime());
    static void scanMediaFile(const QString& fileName);
};

}
