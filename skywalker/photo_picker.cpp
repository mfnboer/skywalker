// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "photo_picker.h"
#include "atproto_image_provider.h"
#include "file_utils.h"
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

}

namespace Skywalker::PhotoPicker {
;
std::tuple<QImage, QString> readImageFd(int fd)
{
    if (fd < 0)
    {
        qWarning() << "Invalid file descriptor";
        return { QImage{}, "Invalid file descriptor (file not found)" };
    }

    QFile file;

    if (!file.open(fd, QFile::OpenModeFlag::ReadOnly, QFile::FileHandleFlag::AutoCloseHandle))
    {
        const QString fileError = file.errorString();
        qWarning() << "Could not open file:" << fileError;
        return { QImage{}, "Could not open file: " + fileError };
    }

    QImageReader reader(&file);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
    {
        QString readError = reader.errorString();
        qWarning() << "Could not read image data:" << readError;
        return { QImage{}, "Could not read image data: " + readError };
    }

    return { img, "" };
}

bool pickPhoto()
{
#ifdef Q_OS_ANDROID
    if (!FileUtils::checkReadMediaPermission())
        return false;

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/QPhotoPicker", "start");
#endif
    return true;
}

QImage loadImage(const QString& imgName)
{
    qDebug() << "Load image:" << imgName;

    if (imgName.startsWith("file://"))
    {
        const QString fileName = imgName.sliced(7);

        if (!FileUtils::checkReadMediaPermission())
        {
            qWarning() << "No permission toe read:" << fileName;
            return {};
        }

        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        QImage img = reader.read();

        if (img.isNull())
            qWarning() << "Failed to read:" << fileName;

        return img;
    }
    else if (imgName.startsWith(QString("image://") + SharedImageProvider::SHARED_IMAGE))
    {
        auto* imgProvider = SharedImageProvider::getProvider(SharedImageProvider::SHARED_IMAGE);
        auto img = imgProvider->getImage(imgName);
        return img;
    }
    else if (imgName.startsWith(QString("image://") + ATProtoImageProvider::DRAFT_IMAGE))
    {
        auto* imgProvider = ATProtoImageProvider::getProvider(ATProtoImageProvider::DRAFT_IMAGE);
        auto img = imgProvider->getImage(imgName);
        return img;
    }

    qWarning() << "Unsupported image name:" << imgName;
    return {};
}

QImage cutRect(const QString& imgName, const QRect& rect)
{
    QImage img = loadImage(imgName);

    if (img.isNull())
        return {};

    return img.copy(rect);
}

QString createBlob(QByteArray& blob, const QString& imgName)
{
    QImage img = loadImage(imgName);

    if (img.isNull())
        return {};

    return createBlob(blob, img, imgName);
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

static QString createPictureFileName()
{
    return QString("SKYWALKER_%1.jpg").arg(FileUtils::createDateTimeName());
}

void savePhoto(const QString& sourceUrl, const std::function<void()>& successCb,
               const std::function<void(const QString&)>& errorCb)
{
    static ImageReader imageReader;

    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    imageReader.getImage(sourceUrl,
        [successCb, errorCb](QImage img){
            if (!FileUtils::checkWriteMediaPermission())
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
