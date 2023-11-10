// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QImage>
#include <QString>

namespace Skywalker {

// Start photo pick selector on Android.
bool pickPhoto();

// Create a binary blob (image/*) for uploading an image.
// The mimetype is returned
// If the width or height is larger than 2000 pixels, it will be scaled down to 2000 pixels.
// The image will be rotated according to EXIF meta data.
// EXIF meta data will be stripped from the blob.
// imgName is "file://..." or "image://..."
QString createBlob(QByteArray& blob, const QString& imgName);
QString createBlob(QByteArray& blob, QImage img, const QString& fileName = "");

QString resolveContentUriToFile(const QString& contentUri);

}
