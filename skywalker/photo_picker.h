// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include <QImage>
#include <QString>
#include <tuple>

namespace Skywalker::PhotoPicker {

std::tuple<QImage, QString /* gif temp file name */, QString /* error */> readImageFd(int fd);

// Start photo pick selector on Android.
bool pickPhoto(bool pickVideo, int maxItems);

QImage loadImage(const QString& imgName);

// Create a binary blob (image/*) for uploading an image.
// { mimetype, image size } is returned
// The image will be rotated according to EXIF meta data.
// EXIF meta data will be stripped from the blob.
// imgName is "file://..." or "image://..."
std::tuple<QString, QSize> createBlob(QByteArray& blob, int maxBytes, const QString& imgName, const QStringList& extraFormats = { "png", "webp" });
std::tuple<QString, QSize> createBlob(QByteArray& blob, int maxBytes, QImage img, const QStringList& extraFormats = { "png", "webp" }, const QString& fileName = "");

QImage cutRect(const QString& imgName, const QRect& rect);

void savePhoto(ImageReader* imageReader, const QString& sourceUrl, bool cache,
               const std::function<void(const QString&)>& successCb,
               const std::function<void(const QString&)>& errorCb);

void copyPhotoToClipboard(ImageReader* imageReader, const QString& sourceUrl,
                          const std::function<void()>& successCb,
                          const std::function<void(const QString&)>& errorCb);

}
