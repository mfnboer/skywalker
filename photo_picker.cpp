// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "photo_picker.h"
#include <QtGlobal>
#include <QBuffer>
#include <QCoreApplication>
#include <QImage>
#include <QImageReader>

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

#if defined(Q_OS_ANDROID)
bool checkReadMediaPermission()
{
    static const QString READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";
    static const QString READ_MEDIA_IMAGES = "android.permission.READ_MEDIA_IMAGES";
    static const QString READ_MEDIA_VISUAL_USER_SELECTED = "android.permission.READ_MEDIA_VISUAL_USER_SELECTED";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion > QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES) && checkPermission(READ_MEDIA_VISUAL_USER_SELECTED);

    if (osVersion >= QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES);

    return checkPermission(READ_EXTERNAL_STORAGE);
}
#endif

}

namespace Skywalker {

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
        uri.object<jstring>()
        );

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
    QString fileName;

    if (imgName.startsWith("file://"))
    {
        fileName = imgName.sliced(7);
    }
    else
    {
        qWarning() << "Unsupported image name:" << imgName;
        return {};
    }

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

}
