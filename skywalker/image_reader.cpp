// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "image_reader.h"
#include "network_holder.h"
#include "photo_picker.h"
#include "skywalker.h"
#include <QImageReader>

namespace Skywalker {

ImageReader::ImageReader(QObject* parent) :
    QObject(parent),
    mNetwork(NetworkHolder::network())
{
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

    return getImageFromWeb(urlString,
        [imageCb](QImage img, const QString&){
            if (imageCb)
                imageCb(img);
        },
        errorCb);
}

static QUrl makeUrl(const QString& urlString)
{
    return QUrl(urlString.startsWith("http") ? urlString : "https://" + urlString);
}

bool ImageReader::getImageFromWeb(const QString& urlString, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb)
{
    qDebug() << "Get image from web:" << urlString;

    QUrl url = makeUrl(urlString);

    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << urlString;
        return false;
    }

    QNetworkReply* reply = sendRequest(url);

    connect(reply, &QNetworkReply::finished, this, [this, reply, imageCb, errorCb]{
            replyFinished(reply, imageCb, errorCb); });

    return true;
}

QNetworkReply* ImageReader::sendRequest(const QUrl& url)
{
    QNetworkRequest request(url);

    // Some image servers don't support HTTP/2, e.g. www.itmagazine.ch
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    // Required for www.itmagazine.ch
    request.setRawHeader("User-Agent", Skywalker::getUserAgentString().toUtf8());

    QNetworkReply* reply = mNetwork->get(request);
    return reply;
}

void ImageReader::replyFinished(QNetworkReply* reply, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb)
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
    {
        const QString format = reader.format();
        qDebug() << "Image format:" << format << "url:" << reply->url();
        imageCb(img, format);
    }
}

void ImageReader::getAsyncImageFromWeb(const QString& urlString, QPointer<ImageReaderResponse> response)
{
    qDebug() << "Get async image from web:" << urlString;

    if (!response)
        return;

    QUrl url = makeUrl(urlString);

    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << urlString;
        QMetaObject::invokeMethod(response, [response]{ response->onError("Invalid lik"); });
    }

    QNetworkReply* reply = sendRequest(url);

    connect(reply, &QNetworkReply::finished, this, [reply, response=QPointer<ImageReaderResponse>(response)]{
        if (reply->error() != QNetworkReply::NoError)
        {
            const QString& error = reply->errorString();
            qWarning() << error;

            if (response)
                QMetaObject::invokeMethod(response, [response, error]{ response->onError(error); });

            return;
        }

        const auto data = reply->readAll();

        if (response)
            QMetaObject::invokeMethod(response, [response, data]{ response->onRawData(data); });
    });
}

void ImageReaderResponse::onRawData(QByteArray data)
{
    QBuffer buffer(&data);
    QImageReader reader(&buffer);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
        onError("Failed to decode data");
    else
        onImage(img, reader.format());

}

}
