// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQuickImageResponse>

namespace Skywalker {

class ImageReaderResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    virtual ~ImageReaderResponse() = default;

    void onRawData(QByteArray data);
    virtual void onError(QString error) = 0;
    virtual void onImage(QImage image, QString format) = 0;
};

class ImageReader : public QObject
{
    Q_OBJECT
public:
    using ImageCb = std::function<void(QImage)>;
    using ImageAndFormatCb = std::function<void(QImage, const QString& format)>;
    using ErrorCb = std::function<void(const QString& error)>;

    explicit ImageReader(QObject* parent = nullptr);
    explicit ImageReader(QNetworkAccessManager* network, QObject* parent = nullptr);

    // Gets file:// image:// http:// https:// images
    bool getImage(const QString& urlString, const ImageCb& imageCb, const ErrorCb& errorCb);

    bool getImageFromWeb(const QString& urlString, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb);

    Q_INVOKABLE void getAsyncImageFromWeb(const QString& urlString, QPointer<ImageReaderResponse> response);

private:
    QNetworkReply* sendRequest(const QUrl& url);
    void replyFinished(QNetworkReply* reply, const ImageAndFormatCb& imageCb, const ErrorCb& errorCb);

    QNetworkAccessManager* mNetwork;
};

}
