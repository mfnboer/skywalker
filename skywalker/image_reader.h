// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Skywalker {

class ImageReader : public QObject
{
public:
    using ImageCb = std::function<void(QImage)>;
    using ImageAndFormatCb = std::function<void(QImage, const QString& format)>;
    using ErrorCb = std::function<void(const QString& error)>;

    explicit ImageReader(QObject* parent = nullptr);
    explicit ImageReader(QNetworkAccessManager* network, QObject* parent = nullptr);

    // Gets file:// image:// http:// https:// images
    bool getImage(const QString& urlString, const ImageCb& imageCb, const ErrorCb& errorCb);

    bool getImageFromWeb(const QString& urlString, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb);

private:
    void replyFinished(QNetworkReply* reply, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb);

    QNetworkAccessManager* mNetwork;
};

}
