// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtQmlIntegration>


namespace Skywalker {

class M3U8Reader : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit M3U8Reader(QObject* parent = nullptr);

    Q_INVOKABLE void getVideoStream(const QString& link, bool firstCall = true);

signals:
    void getVideoStreamOk(QString videoStream);
    void getVideoStreamFailed();

private:
    void reset();
    void extractStream(QNetworkReply* reply);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);
    static QString buildStreamUrl(const QUrl& requestUrl, const QString& stream);

    QNetworkAccessManager mNetwork;
    QNetworkReply* mInProgress = nullptr;
    int mLoopCount = 0; // protect against potential loop
};

}
