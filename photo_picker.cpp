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
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace {

constexpr size_t MAX_IMAGE_BYTES = 1000000;
constexpr int MAX_IMAGE_PIXEL_SIZE = 2000;

bool checkStoragePermission()
{
#if defined(Q_OS_ANDROID)
    static const QString READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";
    auto checkResult = QtAndroidPrivate::checkPermission(READ_EXTERNAL_STORAGE);
    if (checkResult.result() != QtAndroidPrivate::Authorized)
    {
        qDebug() << "Read storage permission check failed.";
        auto requestResult = QtAndroidPrivate::requestPermission(READ_EXTERNAL_STORAGE);
        if (requestResult.result() != QtAndroidPrivate::Authorized)
        {
            qWarning() << "No permission to read storage.";
            return false;
        }
    }
#endif
    return true;
}

}

namespace Skywalker {

void pickPhoto()
{
#ifdef Q_OS_ANDROID
    if (!checkStoragePermission())
        return;

    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/QPhotoPicker",
                                       "start");
#endif
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
    return {};
#endif
}

QByteArray createBlob(const QString& fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    qDebug() << "Original image:" << fileName << "geometry:" << img.size() << "bytes:" << img.sizeInBytes();

    if (std::max(img.width(), img.height()) > MAX_IMAGE_PIXEL_SIZE)
    {
        if (img.width() > img.height())
            img = img.scaledToWidth(MAX_IMAGE_PIXEL_SIZE, Qt::SmoothTransformation);
        else
            img = img.scaledToHeight(MAX_IMAGE_PIXEL_SIZE, Qt::SmoothTransformation);
    }

    QByteArray blob;
    QBuffer buffer(&blob);
    buffer.open(QIODevice::WriteOnly);

    if (!img.save(&buffer, "jpg"))
    {
        qWarning() << "Failed to write blob:" << fileName;
        blob.clear();
    }

    qDebug() << "Blob size:" << blob.size();
    if (blob.size() > MAX_IMAGE_BYTES)
    {
        qWarning() << "Image too large:" << fileName << "blob bytes:" << blob.size();
        blob.clear();
    }

    return blob;
}

}
