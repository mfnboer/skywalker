// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QImage>
#include <QString>

namespace Skywalker {

// Start photo pick selector on Android.
void pickPhoto();

// Create a binary blob (image/*) for uploading an image.
// The mimetype is returned
// If the width or height is larger than 2000 pixels, it will be scaled down to 2000 pixels.
// The image will be rotated according to EXIF meta data.
// EXIF meta data will be stripped from the blob.
QString createBlob(QByteArray& blob, const QString& fileName);
QString createBlob(QByteArray& blob, QImage img, const QString& fileName = "");

QString resolveContentUriToFile(const QString& contentUri);

}
