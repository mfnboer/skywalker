// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "photo_picker.h"
#include "atproto_image_provider.h"
#include "file_utils.h"
#include "image_reader.h"
#include "shared_image_provider.h"
#include "temp_file_holder.h"
#include <QtGlobal>
#include <QBuffer>
#include <QCoreApplication>
#include <QClipboard>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QStandardPaths>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QOperatingSystemVersion>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace {
constexpr int MAX_IMAGE_PIXEL_SIZE = 2000;
}

namespace Skywalker::PhotoPicker {
;
std::tuple<QImage, QString, QString> readImageFd(int fd)
{
    QString gifTempFileName;

    if (fd < 0)
    {
        qWarning() << "Invalid file descriptor";
        return { QImage{}, gifTempFileName, "Invalid file descriptor (file not found)" };
    }

    QFile file;

    if (!file.open(fd, QFile::OpenModeFlag::ReadOnly, QFile::FileHandleFlag::AutoCloseHandle))
    {
        const QString fileError = file.errorString();
        qWarning() << "Could not open file:" << fileError;
        return { QImage{}, gifTempFileName, "Could not open file: " + fileError };
    }

    QImageReader reader(&file);
    qDebug() << "Input image format:" << reader.format();
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
    {
        QString readError = reader.errorString();
        qWarning() << "Could not read image data:" << readError;
        return { QImage{}, gifTempFileName, "Could not read image data: " + readError };
    }

    if (reader.format() == "gif")
    {
        file.seek(0);
        auto gifTempFile = FileUtils::createTempFile(file, "gif");

        if (gifTempFile)
        {
            gifTempFileName = gifTempFile->fileName();
            TempFileHolder::instance().put(std::move(gifTempFile));
        }
        else
        {
            qWarning() << "Could not create temp file for GIF";
        }
    }

    return { img, gifTempFileName, "" };
}

bool pickPhoto(bool pickVideo)
{
#ifdef Q_OS_ANDROID
    if (!FileUtils::checkReadMediaPermission())
        return false;

    jboolean jVideo = pickVideo;
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/QPhotoPicker", "start", "(Z)V", jVideo);
#endif
    Q_UNUSED(pickVideo)
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
            qWarning() << "No permission to read:" << fileName;
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

std::tuple<QString, QSize> createBlob(QByteArray& blob, const QString& imgName)
{
    QImage img = loadImage(imgName);

    if (img.isNull())
        return {};

    return createBlob(blob, img, imgName);
}

std::tuple<QString, QSize> createBlob(QByteArray& blob, QImage img, const QString& name)
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

        if (blob.size() > ATProto::AppBskyEmbed::Image::MAX_BYTES)
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

    return { mimeType, img.size() };
}

static QString createPictureFileName()
{
    return QString("SKYWALKER_%1.jpg").arg(FileUtils::createDateTimeName());
}

void savePhoto(const QString& sourceUrl, bool cache,
               const std::function<void(const QString&)>& successCb,
               const std::function<void(const QString&)>& errorCb)
{
    static ImageReader imageReader;

    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    imageReader.getImage(sourceUrl,
        [cache, successCb, errorCb](QImage img){
            QString picturesPath;

            if (cache)
            {
                picturesPath = FileUtils::getCachePath("shared");
            }
            else
            {
                if (!FileUtils::checkWriteMediaPermission())
                {
                    errorCb(QObject::tr("No permission to save pictures"));
                    return;
                }

                picturesPath = FileUtils::getPicturesPath();
            }

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

            if (!cache)
                FileUtils::scanMediaFile(fileName);

            successCb(fileName);
        },
        [errorCb](const QString& error){
            qWarning() << "Failed get get image:" << error;
            errorCb(error);
        });
}

void copyPhotoToClipboard(const QString& sourceUrl,
                          const std::function<void()>& successCb,
                          const std::function<void(const QString&)>& errorCb)
{
    static ImageReader imageReader;

    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    imageReader.getImage(sourceUrl,
        [successCb, errorCb](QImage img){
            QClipboard *clipboard = QGuiApplication::clipboard();
            clipboard->setImage(img);
            successCb();
        },
        [errorCb](const QString& error){
            qWarning() << "Failed get get image:" << error;
            errorCb(error);
        });
}

}
