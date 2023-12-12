// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "photo_picker.h"
#include "image_reader.h"
#include "shared_image_provider.h"
#include <QtGlobal>
#include <QBuffer>
#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QOperatingSystemVersion>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace {

constexpr qsizetype MAX_IMAGE_BYTES = 1000000;
constexpr int MAX_IMAGE_PIXEL_SIZE = 2000;

#if defined(Q_OS_ANDROID)
bool checkPermission(const QString& permission)
{
    auto checkResult = QtAndroidPrivate::checkPermission(permission);
    if (checkResult.result() != QtAndroidPrivate::Authorized)
    {
        qDebug() << "Permission check failed:" << permission;
        auto requestResult = QtAndroidPrivate::requestPermission(permission);

        if (requestResult.result() != QtAndroidPrivate::Authorized)
        {
            qWarning() << "No permission:" << permission;
            return false;
        }
    }

    return true;
}
#endif

}

namespace Skywalker {

bool checkReadMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";
    static const QString READ_MEDIA_IMAGES = "android.permission.READ_MEDIA_IMAGES";
    static const QString READ_MEDIA_VISUAL_USER_SELECTED = "android.permission.READ_MEDIA_VISUAL_USER_SELECTED";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion > QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES) && checkPermission(READ_MEDIA_VISUAL_USER_SELECTED);

    if (osVersion >= QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES);

    return checkPermission(READ_EXTERNAL_STORAGE);
#else
    return true;
#endif
}

bool checkWriteMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString WRITE_EXTERNAL_STORAGE = "android.permission.WRITE_EXTERNAL_STORAGE";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion < QOperatingSystemVersion::Android11)
        return checkPermission(WRITE_EXTERNAL_STORAGE);
#endif
    return true;
}

int openContentUri(const QString& contentUri)
{
#if defined(Q_OS_ANDROID)
    QJniObject uri = QJniObject::fromString(contentUri);

    int fd = QJniObject::callStaticMethod<int>(
        "com/gmail/mfnboer/FileUtils",
        "openContentUriString",
        "(Ljava/lang/String;)I",
        uri.object<jstring>());

    return fd;
#else
    qWarning() << "Cannot handle content-URI:" << contentUri;
    return -1;
#endif
}

QImage readImageFd(int fd)
{
    if (fd < 0)
    {
        qWarning() << "Invalid file descriptor";
        return {};
    }

    QFile file;

    if (!file.open(fd, QFile::OpenModeFlag::ReadOnly, QFile::FileHandleFlag::AutoCloseHandle))
    {
        qWarning() << "Could not open file";
        return {};
    }

    QImageReader reader(&file);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
    {
        qWarning() << "Could not read image data.";
        return {};
    }

    return img;
}

bool pickPhoto()
{
#ifdef Q_OS_ANDROID
    if (!checkReadMediaPermission())
        return false;

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/QPhotoPicker", "start");
#endif
    return true;
}

QString resolveContentUriToFile(const QString &contentUriString) {
#ifdef Q_OS_ANDROID
    QJniObject uri = QJniObject::fromString(contentUriString);

    // Call the Java method
    QJniObject result = QJniObject::callStaticObjectMethod(
        "com/gmail/mfnboer/FileUtils",
        "resolveContentUriToFile",
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

QString createBlob(QByteArray& blob, const QString& imgName)
{
    if (imgName.startsWith("file://"))
    {
        const QString fileName = imgName.sliced(7);

        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        QImage img = reader.read();

        if (img.isNull())
        {
            qWarning() << "Failed to read:" << fileName;
            return {};
        }

        return createBlob(blob, img, imgName);
    }
    else if (imgName.startsWith("image://"))
    {
        auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        auto img = imgProvider->getImage(imgName);

        if (img.isNull())
            return {};

        return createBlob(blob, img, imgName);
    }

    qWarning() << "Unsupported image name:" << imgName;
    return {};
}

QString createBlob(QByteArray& blob, QImage img, const QString& name)
{
    qDebug() << "Original image:" << name << "geometry:" << img.size() << "bytes:" << img.sizeInBytes();

    if (std::max(img.width(), img.height()) > MAX_IMAGE_PIXEL_SIZE)
    {
        if (img.width() > img.height())
            img = img.scaledToWidth(MAX_IMAGE_PIXEL_SIZE, Qt::SmoothTransformation);
        else
            img = img.scaledToHeight(MAX_IMAGE_PIXEL_SIZE, Qt::SmoothTransformation);
    }



    const char* format = "jpg";
    QString mimeType = "image/jpeg";

    if (name.endsWith(".png", Qt::CaseInsensitive))
    {
        format = "png";
        mimeType = "image/png";
    }

    int quality = 75;

    while (quality > 0)
    {
        QBuffer buffer(&blob);
        buffer.open(QIODevice::WriteOnly);

        if (!img.save(&buffer, format, quality))
        {
            qWarning() << "Failed to write blob:" << name << "format:" << format;
            blob.clear();
            break;
        }

        qDebug() << "Blob created, bytes:" << blob.size() << "format:" << format << "mimetype:" << mimeType << "quality:" << quality;

        if (blob.size() > MAX_IMAGE_BYTES)
        {
            qDebug() << "Image too large:" << name << "blob bytes:" << blob.size();
            blob.clear();
            quality -= 25;

            if (quality <= 0 && mimeType == "image/png")
            {
                // PNG compression does not compress well. Try JPG
                format = "jpg";
                mimeType = "image/jpeg";
                quality = 75;
            }

            continue;
        }

        break;
    }

    return mimeType;
}

static void scanMediaFile(const QString& fileName)
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

static QString getPicturesPath()
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

static QString createDateTimeName()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
}

static QString createPictureFileName()
{
    return QString("SKYWALKER_%1.jpg").arg(createDateTimeName());
}

void savePhoto(const QString& sourceUrl, const std::function<void()>& successCb,
               const std::function<void(const QString&)>& errorCb)
{
    static ImageReader imageReader;

    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    imageReader.getImage(sourceUrl,
        [successCb, errorCb](QImage img){
            if (!checkWriteMediaPermission())
            {
                errorCb(QObject::tr("No permission to save pictures"));
                return;
            }

            const QString picturesPath = getPicturesPath();

            if (picturesPath.isEmpty())
            {
                qWarning() << "Cannot get pictures path.";
                errorCb(QObject::tr("No location to save pictures"));
                return;
            }

            const QString fileName = QString("%1/%2").arg(picturesPath, createPictureFileName());

            if (!img.save(fileName))
            {
                qWarning() << "Failed to save file:" << fileName;
                errorCb(QObject::tr("Failed to save picture"));
            }

            scanMediaFile(fileName);
            successCb();
        },
        [errorCb](const QString& error){
            qWarning() << "Failed get get image:" << error;
            errorCb(error);
        });
}

}
