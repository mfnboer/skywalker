// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QtQmlIntegration>


namespace Skywalker {

// Load Bluesky video stream into a temp file to work around a bug in the
// Qt live streamer: https://bugreports.qt.io/browse/QTBUG-128908
class M3U8Reader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged FINAL)
    QML_ELEMENT

public:
    explicit M3U8Reader(QObject* parent = nullptr);

    bool isLoading() const { return mLoading; }
    void setLoading(bool loading);
    Q_INVOKABLE void getVideoStream(const QString& link, bool firstCall = true);
    Q_INVOKABLE void loadStream();

signals:
    void getVideoStreamOk(int durationMs);
    void getVideoStreamError();
    void loadStreamOk(QString videoStream);
    void loadStreamError();
    void loadingChanged();

private:
    void reset();
    void extractStream(QNetworkReply* reply);
    void requestFailed(QNetworkReply* reply, int errCode);
    void requestSslFailed(QNetworkReply* reply);
    static QString buildStreamUrl(const QUrl& requestUrl, const QString& stream);

    void loadStream(QNetworkReply* reply);
    void loadStreamFailed(QNetworkReply* reply, int errCode);
    void loadStreamSslFailed(QNetworkReply* reply);

    QNetworkAccessManager mNetwork;
    QNetworkReply* mInProgress = nullptr;
    int mLoopCount = 0; // protect against potential loop
    QStringList mStreamSegments;
    std::unique_ptr<QTemporaryFile> mStream;
    bool mLoading = false;
};

}
