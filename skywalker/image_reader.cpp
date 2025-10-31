// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "image_reader.h"
#include "photo_picker.h"
#include <QImageReader>

namespace Skywalker {

ImageReader::ImageReader(QObject* parent) :
    QObject(parent)
{
    mNetwork = new QNetworkAccessManager(this);
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
}

ImageReader::ImageReader(QNetworkAccessManager* network, QObject* parent) :
    QObject(parent),
    mNetwork(network)
{
    Q_ASSERT(mNetwork);
}

bool ImageReader::getImage(const QString& urlString, const ImageCb& imageCb, const ErrorCb& errorCb)
{
    qDebug() << "Get image:" << urlString;

    if (urlString.startsWith("file://") || urlString.startsWith("image://"))
    {
        auto img = PhotoPicker::loadImage(urlString);

        if (!img.isNull())
            imageCb(img);
        else
            errorCb("Failed to load image");

        return true;
    }

    return getImageFromWeb(urlString, imageCb, errorCb);
}

bool ImageReader::getImageFromWeb(const QString& urlString, const ImageCb& imageCb, const ErrorCb& errorCb)
{
    qDebug() << "Get image from web:" << urlString;

    QUrl url(urlString.startsWith("http") ? urlString : "https://" + urlString);
    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << urlString;
        return false;
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, imageCb, errorCb]{
            replyFinished(reply, imageCb, errorCb); });

    return true;
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
