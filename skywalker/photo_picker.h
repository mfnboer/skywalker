// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QImage>
#include <QString>
#include <tuple>

namespace Skywalker::PhotoPicker {

std::tuple<QImage, QString /* gif temp file name */, QString /* error */> readImageFd(int fd);

// Start photo pick selector on Android.
bool pickPhoto(bool pickVideo = false);

QImage loadImage(const QString& imgName);

// Create a binary blob (image/*) for uploading an image.
// { mimetype, image size } is returned
// If the width or height is larger than 2000 pixels, it will be scaled down to 2000 pixels.
// The image will be rotated according to EXIF meta data.
// EXIF meta data will be stripped from the blob.
// imgName is "file://..." or "image://..."
std::tuple<QString, QSize> createBlob(QByteArray& blob, const QString& imgName);
std::tuple<QString, QSize> createBlob(QByteArray& blob, QImage img, const QString& fileName = "");

QImage cutRect(const QString& imgName, const QRect& rect);

void savePhoto(const QString& sourceUrl, const std::function<void()>& successCb,
               const std::function<void(const QString&)>& errorCb);

}
