// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "m3u8_reader.h"
#include "file_utils.h"
#include "m3u8_parser.h"

namespace Skywalker {

M3U8Reader::M3U8Reader(QObject* parent) :
    QObject(parent)
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
}

void M3U8Reader::getVideoStream(const QString& link, bool firstCall)
{
    if (firstCall)
        mLoopCount = 5;
    else
        --mLoopCount;

    qDebug() << "Get video stream:" << link << "firstCall:" << firstCall << "loopCount:" << mLoopCount;

    if (mLoopCount <= 0)
    {
        qWarning() << "Loop detected:" << link;
        emit getVideoStreamFailed();
        return;
    }

    if (mInProgress)
    {
        mInProgress->abort();
        mInProgress = nullptr;
    }

    QUrl url(link);
    if (!url.isValid())
    {
        qWarning() << "Invalid link:" << link;
        emit getVideoStreamFailed();
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);
    mInProgress = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply]{ extractStream(reply); });
    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](auto errCode){ requestFailed(reply, errCode); });
    connect(reply, &QNetworkReply::sslErrors, this, [this, reply]{ requestSslFailed(reply); });
}

void M3U8Reader::extractStream(QNetworkReply* reply)
{
    mInProgress = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        requestFailed(reply, reply->error());
        return;
    }

    const auto data = reply->readAll();
    M3U8Parser parser;

    const auto streamType = parser.parse(data);

    if (streamType == M3U8StreamType::VIDEO)
    {
        if (parser.getStreamSegments().isEmpty())
        {
            qWarning() << "Empty stream:" << reply->request().url();
            emit getVideoStreamFailed();
            return;
        }

        qDebug() << "Got video stream:" << parser.getStreamSegments();

        for (const QString& segment : parser.getStreamSegments())
        {
            const QString streamUrl = buildStreamUrl(reply->request().url(), segment);
            mStreamSegments.push_back(streamUrl);
        }

        mStream = FileUtils::makeTempFile("ts");

        if (!mStream)
        {
            qWarning() << "Failed to create temp file";
            emit getVideoStreamFailed();
        }


        qDebug() << "Created temp file:" << mStream->fileName();
        loadStream();
        return;
    }

    Q_ASSERT(streamType == M3U8StreamType::PLAYLIST);
    QString stream = parser.getStream360();

    if (stream.isEmpty())
        stream = parser.getStream720();

    if (stream.isEmpty())
    {
        qWarning() << "Cannot find stream:" << reply->request().url();
        emit getVideoStreamFailed();
        return;
    }

    qDebug() << "Extracted stream:" << stream;
    const QString streamUrl = buildStreamUrl(reply->request().url(), stream);
    getVideoStream(streamUrl);
}

QString M3U8Reader::buildStreamUrl(const QUrl& requestUrl, const QString& stream)
{
    const auto& urlBase = requestUrl.adjusted(QUrl::RemoveFilename | QUrl::RemoveQuery);
    const QString streamUrl = urlBase.toString() + stream;
    qDebug() << "Stream URL:" << streamUrl;
    return streamUrl;
}

void M3U8Reader::requestFailed(QNetworkReply* reply, int errCode)
{
    mInProgress = nullptr;
    qDebug() << "Failed to get link:" << reply->request().url();
    qDebug() << "Error:" << errCode << reply->errorString();
    qDebug() << reply->readAll();
    emit getVideoStreamFailed();
}

void M3U8Reader::requestSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to get link:" << reply->request().url();
    emit getVideoStreamFailed();
}

void M3U8Reader::loadStream()
{
    if (mStreamSegments.isEmpty())
    {
        qDebug() << "No more segments to load";
        mStream->close();
        QUrl url = QUrl::fromLocalFile(mStream->fileName());
        emit getVideoStreamOk(url.toString());
        return;
    }

    QUrl url(mStreamSegments.first());
    if (!url.isValid())
    {
        qWarning() << "Invalid segment URL:" << mStreamSegments.first();
        emit getVideoStreamFailed();
        return;
    }

    mStreamSegments.pop_front();
    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]{ loadStream(reply); });
    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](auto errCode){ loadStreamFailed(reply, errCode); });
    connect(reply, &QNetworkReply::sslErrors, this, [this, reply]{ loadStreamSslFailed(reply); });
}

void M3U8Reader::loadStream(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        loadStreamFailed(reply, reply->error());
        return;
    }

    const auto data = reply->readAll();

    if (mStream->write(data) < 0)
    {
        qWarning() << "Failed to save stream into tempfile:" << mStream->fileName();
        emit getVideoStreamFailed();
        return;
    }
    else
    {
        qDebug() << "Saved:" << reply->request().url() << "to:" << mStream->fileName();
        mStream->flush();
        loadStream();
    }
}

void M3U8Reader::loadStreamFailed(QNetworkReply* reply, int errCode)
{
    mInProgress = nullptr;
    qDebug() << "Failed to load stream segment:" << reply->request().url();
    qDebug() << "Error:" << errCode << reply->errorString();
    qDebug() << reply->readAll();
    emit getVideoStreamFailed();
}

void M3U8Reader::loadStreamSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to load stream segment:" << reply->request().url();
    emit getVideoStreamFailed();
}

}
