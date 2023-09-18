// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "image_reader.h"
#include <QImageReader>

namespace Skywalker {

ImageReader::ImageReader() :
    QObject()
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
}

void ImageReader::getImage(const QString& urlString, const ImageCb& imageCb, const ErrorCb& errorCb)
{
    qDebug() << "Get image:" << urlString;

    QUrl url(urlString.startsWith("http") ? urlString : "https://" + urlString);
    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << urlString;
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, imageCb, errorCb]{
            replyFinished(reply, imageCb, errorCb); });
}

void ImageReader::replyFinished(QNetworkReply* reply, const ImageCb& imageCb, const ErrorCb& errorCb)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        const QString& error = reply->errorString();
        qWarning() << error;

        if (errorCb)
            errorCb(error);

        return;
    }

    QImageReader reader(reply);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
    {
        qWarning() << "Failed to read:" << reply->request().url();
        if (errorCb)
            errorCb(tr("Could not read image"));

        return;
    }

    if (imageCb)
        imageCb(img);
}

}
