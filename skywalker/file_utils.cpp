// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "file_utils.h"
#include "temp_file_holder.h"
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

#ifdef Q_OS_ANDROID
#include "android_utils.h"
#include <QJniObject>
#include <QOperatingSystemVersion>
#endif

namespace {

constexpr char const* APP_DATA_SUB_DIR = "skywalker";

}

namespace Skywalker {

bool FileUtils::checkReadMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";
    static const QString READ_MEDIA_IMAGES = "android.permission.READ_MEDIA_IMAGES";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 15))
        return isPhotoPickerAvailable() || AndroidUtils::checkPermission(READ_MEDIA_IMAGES);

    if (osVersion >= QOperatingSystemVersion::Android13)
        return AndroidUtils::checkPermission(READ_MEDIA_IMAGES);

    return AndroidUtils::checkPermission(READ_EXTERNAL_STORAGE);
#else
    return true;
#endif
}

bool FileUtils::checkWriteMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString WRITE_EXTERNAL_STORAGE = "android.permission.WRITE_EXTERNAL_STORAGE";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion < QOperatingSystemVersion::Android11)
        return AndroidUtils::checkPermission(WRITE_EXTERNAL_STORAGE);
#endif
    return true;
}

bool FileUtils::isPhotoPickerAvailable()
{
#if defined(Q_OS_ANDROID)
    bool available = QJniObject::callStaticMethod<bool>(
        "com/gmail/mfnboer/QPhotoPicker",
        "isPhotoPickerAvailable",
        "()Z");

    qDebug() << "Photo picker available:" << available;
    return available;
#else
    return false;
#endif
}

QString FileUtils::getAppDataPath(const QString& subDir)
{
#if defined(Q_OS_ANDROID)
    QJniObject jsSubDir = QJniObject::fromString(QString("%1/%2").arg(APP_DATA_SUB_DIR, subDir));
    auto pathObj = QJniObject::callStaticMethod<jstring>(
        "com/gmail/mfnboer/FileUtils",
        "getAppDataPath",
        "(Ljava/lang/String;)Ljava/lang/String;",
        jsSubDir.object<jstring>());

    if (!pathObj.isValid())
    {
        qWarning() << "Could not get app data path:" << subDir;
        return {};
    }

    return pathObj.toString();
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString appDataPath = path + "/" + APP_DATA_SUB_DIR + "/" + subDir;

    if (!QDir().mkpath(appDataPath))
    {
        qWarning() << "Failed to create path:" << appDataPath;
        return {};
    }

    return appDataPath;
#endif
}

QString FileUtils::getPicturesPath()
{
#if defined(Q_OS_ANDROID)
    auto pathObj = QJniObject::callStaticMethod<jstring>("com/gmail/mfnboer/FileUtils",
                                                         "getPicturesPath",
                                                         "()Ljava/lang/String;");

    if (!pathObj.isValid())
    {
        qWarning() << "Invalid path object.";
        return {};
    }

    return pathObj.toString();
#else
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
#endif
}

QString FileUtils::getPicturesPath(const QString& subDir)
{
    Q_ASSERT(!subDir.isEmpty());
#if defined(Q_OS_ANDROID)
    auto jsSubDir = QJniObject::fromString(subDir);
    auto pathObj = QJniObject::callStaticMethod<jstring>(
        "com/gmail/mfnboer/FileUtils",
        "getPicturesPath",
        "(Ljava/lang/String;)Ljava/lang/String;",
        jsSubDir.object<jstring>());

    if (!pathObj.isValid())
    {
        qWarning() << "Failed to create pictures path:" << subDir;
        return {};
    }

    const QString picPath = pathObj.toString();
    qDebug() << "Pictures path:" << picPath;
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString picPath = path + "/" + subDir;

    if (!QDir().mkpath(picPath))
    {
        qWarning() << "Failed to create pictures path:" << picPath;
        return {};
    }
#endif
    return picPath;
}

QString FileUtils::getMoviesPath()
{
#if defined(Q_OS_ANDROID)
    auto pathObj = QJniObject::callStaticMethod<jstring>("com/gmail/mfnboer/FileUtils",
                                                         "getMoviesPath",
                                                         "()Ljava/lang/String;");

    if (!pathObj.isValid())
    {
        qWarning() << "Invalid path object.";
        return {};
    }

    return pathObj.toString();
#else
    return QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
#endif
}

QString FileUtils::getCachePath(const QString& subDir)
{
    Q_ASSERT(!subDir.isEmpty());
#if defined(Q_OS_ANDROID)
    auto jsSubDir = QJniObject::fromString(subDir);
    auto pathObj = QJniObject::callStaticMethod<jstring>(
        "com/gmail/mfnboer/FileUtils",
        "getCachePath",
        "(Ljava/lang/String;)Ljava/lang/String;",
        jsSubDir.object<jstring>());

    if (!pathObj.isValid())
    {
        qWarning() << "Failed to create cache path:" << subDir;
        return {};
    }

    const QString cachePath = pathObj.toString();
    qDebug() << "Cache path:" << cachePath;
    return cachePath;
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString cachePath = path + "/" + APP_DATA_SUB_DIR + "/" + subDir;

    if (!QDir().mkpath(cachePath))
    {
        qWarning() << "Failed to create path:" << cachePath;
        return {};
    }

    return cachePath;
#endif
}

static QString fileModeToString(FileUtils::FileMode mode)
{
    switch (mode)
    {
    case FileUtils::FileMode::READ_ONLY:
        return "r";
    case FileUtils::FileMode::WRITE_ONLY:
        return "wt";
    }

    qWarning() << "Unknown file mode:" << (int)mode;
    return "r";
}

int FileUtils::openContentUri(const QString& contentUri, FileMode mode)
{
#if defined(Q_OS_ANDROID)
    QJniObject uri = QJniObject::fromString(contentUri);
    QJniObject jMode = QJniObject::fromString(fileModeToString(mode));

    int fd = QJniObject::callStaticMethod<int>(
        "com/gmail/mfnboer/FileUtils",
        "openContentUriString",
        "(Ljava/lang/String;Ljava/lang/String;)I",
        uri.object<jstring>(),
        jMode.object<jstring>());

    return fd;
#else
    qWarning() << "Cannot handle content-URI:" << contentUri << "mode:" << (int)mode << fileModeToString(mode);
    return -1;
#endif
}

bool FileUtils::deleteContentUri(const QString& contentUri)
{
#if defined(Q_OS_ANDROID)
    QJniObject uri = QJniObject::fromString(contentUri);
    bool deleted = QJniObject::callStaticMethod<bool>(
        "com/gmail/mfnboer/FileUtils",
        "deleteContentUriString",
        "(Ljava/lang/String;)Z",
        uri.object<jstring>());

    return deleted;
#else
    qWarning() << "Cannot handle content-URI:" << contentUri;
    return false;
#endif
}

std::unique_ptr<QFile> FileUtils::openFile(const QUrl& fileUri, FileMode mode)
{
#if defined(Q_OS_ANDROID)
    int fd = openContentUri(fileUri.toString(), mode);

    if (fd < 0)
    {
        qWarning() << "Invalid file descriptor";
        return nullptr;
    }

    auto file = std::make_unique<QFile>();
    const auto openMode = mode == FileMode::WRITE_ONLY ? QIODevice::WriteOnly : QIODevice::ReadOnly;

    if (!file->open(fd, openMode, QFile::FileHandleFlag::AutoCloseHandle))
    {
        qWarning() << "Cannot create file:" << fileUri;
        return nullptr;
    }

    return file;
#else
    const QString fileName = fileUri.toLocalFile();
    auto file = std::make_unique<QFile>(fileName);
    const auto openMode = mode == FileMode::WRITE_ONLY ? QIODevice::WriteOnly : QIODevice::ReadOnly;

    if (!file->open(openMode))
    {
        qWarning() << "Cannot create file:" << fileName;
        return nullptr;
    }

    return file;
#endif
}

QString FileUtils::resolveContentUriToFileName(const QString &contentUriString) {
#ifdef Q_OS_ANDROID
    QJniObject uri = QJniObject::fromString(contentUriString);

    // Call the Java method
    QJniObject result = QJniObject::callStaticObjectMethod(
        "com/gmail/mfnboer/FileUtils",
        "resolveContentUriToFileName",
        "(Ljava/lang/String;)Ljava/lang/String;",
        uri.object<jstring>());

    if (!result.isValid())
    {
        qWarning() << "Could not resolve content-uri:" << contentUriString;
        return {};
    }

    return result.toString();
#else
    Q_UNUSED(contentUriString)
    return {};
#endif
}

std::unique_ptr<QTemporaryFile> FileUtils::makeTempFile(const QString& fileExtension, bool cache)
{
    const QString nameTemplate = TempFileHolder::getNameTemplate(fileExtension, cache);
    auto tmpFile = std::make_unique<QTemporaryFile>(nameTemplate);

    if (!tmpFile->open())
    {
        const QString fileError = tmpFile->errorString();
        qWarning() << "Failed to open tmp file:" << fileError;
        return nullptr;
    }

    return tmpFile;
}

std::unique_ptr<QTemporaryFile> FileUtils::createTempFile(const QString& fileUri, const QString& fileExtension)
{
    qDebug() << "Create temp file for:" << fileUri << "ext:" << fileExtension;

    if (!fileUri.startsWith("file://"))
    {
        qWarning() << "Unknow uri scheme:" << fileUri;
        return nullptr;
    }

    const auto fileName = fileUri.sliced(7);
    QFile file(fileName);
    return createTempFile(file, fileExtension);
}

std::unique_ptr<QTemporaryFile> FileUtils::createTempFile(int fd, const QString& fileExtension)
{
    QFile file;

    if (!file.open(fd, QFile::ReadOnly))
    {
        const QString fileError = file.errorString();
        qWarning() << "Failed to open file:" << fileError;
        return nullptr;
    }

    return createTempFile(file, fileExtension);
}

std::unique_ptr<QTemporaryFile> FileUtils::createTempFile(QFile& file, const QString& fileExtension)
{
    qDebug() << "Create temp file, input size:" << file.size();

    if (!file.isOpen())
    {
        if (!file.open(QFile::ReadOnly))
        {
            const QString fileError = file.errorString();
            qWarning() << "Failed to open file:" << fileError;
            return nullptr;
        }
    }

    auto tmpFile = makeTempFile(fileExtension);

    if (!tmpFile)
        return nullptr;

    if (tmpFile->write(file.readAll()) < 0)
    {
        const QString fileError = tmpFile->errorString();
        qWarning() << "Failed to write file to tmp file:" << fileError;
        return nullptr;
    }

    tmpFile->flush();
    tmpFile->close();

    qDebug() << "Created temp file:" << tmpFile->fileName() << tmpFile->size();
    return tmpFile;
}

QString FileUtils::createDateTimeName(QDateTime timestamp)
{
    return timestamp.toString("yyyyMMddhhmmss");
}

void FileUtils::scanMediaFile(const QString& fileName)
{
#if defined(Q_OS_ANDROID)
    auto jsFileName = QJniObject::fromString(fileName);
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/FileUtils",
                                       "scanMediaFile",
                                       "(Ljava/lang/String;)V",
                                       jsFileName.object<jstring>());
#else
    qDebug() << "No need to scan media:" << fileName;
#endif
}

}
