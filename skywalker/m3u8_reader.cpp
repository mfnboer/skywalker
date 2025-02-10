// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "m3u8_reader.h"
#include "file_utils.h"
#include "m3u8_parser.h"
#include "network_utils.h"

namespace Skywalker {

static constexpr int HD_BANDWIDTH_THRESHOLD_KBPS = 2000;

M3U8Reader::M3U8Reader(QObject* parent) :
    QObject(parent)
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(10000);
}

void M3U8Reader::setVideoQuality(QEnums::VideoQuality quality)
{
    if (quality != mVideoQuality)
    {
        mVideoQuality = quality;
        emit videoQualityChanged();
    }
}

void M3U8Reader::setLoading(bool loading)
{
    if (loading != mLoading)
    {
        mLoading = loading;
        emit loadingChanged();
    }
}

void M3U8Reader::setResolution()
{
    if (mVideoQuality == QEnums::VIDEO_QUALITY_SD ||
        (mVideoQuality == QEnums::VIDEO_QUALITY_HD_WIFI && !NetworkUtils::isUnmetered()) ||
        NetworkUtils::getBandwidthKbps() < HD_BANDWIDTH_THRESHOLD_KBPS)
    {
        mResolution = STREAM_RESOLUTION_360;
    }
    else
    {
        mResolution = STREAM_RESOLUTION_720;
    }

    qDebug() << "Resolution:" << mResolution;
}

void M3U8Reader::getVideoStream(const QString& link, bool firstCall)
{   
    if (firstCall)
    {
        setResolution();
        mLoopCount = 5;
    }
    else
    {
        --mLoopCount;
    }

    qDebug() << "Get video stream:" << link << "firstCall:" << firstCall << "loopCount:" << mLoopCount;

    if (mLoopCount <= 0)
    {
        qWarning() << "Loop detected:" << link;
        emit getVideoStreamError();
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
        emit getVideoStreamError();
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

    if (streamType == M3U8StreamType::ERROR)
    {
        emit getVideoStreamError();
        return;
    }

    if (streamType == M3U8StreamType::VIDEO)
    {
        if (parser.getStreamSegments().isEmpty())
        {
            qWarning() << "Empty stream:" << reply->request().url();
            emit getVideoStreamError();
            return;
        }

        qDebug() << "Got video stream:" << parser.getStreamSegments();

        for (const QString& segment : parser.getStreamSegments())
        {
            const QString streamUrl = buildStreamUrl(reply->request().url(), segment);
            mStreamSegments.push_back(streamUrl);
        }

        emit getVideoStreamOk(parser.getStreamDurationSeconds() * 1000);
        return;
    }

    Q_ASSERT(streamType == M3U8StreamType::PLAYLIST);
    QString stream = mResolution == STREAM_RESOLUTION_360 ? parser.getStream360() : parser.getStream720();

    if (stream.isEmpty())
        stream = mResolution == STREAM_RESOLUTION_360 ? parser.getStream720() : parser.getStream360();

    if (stream.isEmpty())
    {
        qWarning() << "Cannot find stream:" << reply->request().url();
        emit getVideoStreamError();
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
    emit getVideoStreamError();
}

void M3U8Reader::requestSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to get link:" << reply->request().url();
    emit getVideoStreamError();
}

void M3U8Reader::loadStream(const QString& fileName)
{
    if (!mStream)
    {
        if (mStreamSegments.isEmpty())
        {
            qWarning() << "Stream is not yet set";
            return;
        }

        if (fileName.isEmpty())
        {
            mStream = FileUtils::makeTempFile("ts", true);

            if (!mStream)
            {
                qWarning() << "Failed to create temp file";
                emit loadStreamError();
                return;
            }

            qDebug() << "Created temp file:" << mStream->fileName();
        }
        else
        {
            mStream = std::make_unique<QFile>(fileName);

            if (!mStream->open(QFile::WriteOnly))
            {
                qWarning() << "Cannot open file:" << fileName;
                emit loadStreamError();
                return;
            }
        }
    }

    setLoading(true);

    if (mStreamSegments.isEmpty())
    {
        qDebug() << "No more segments to load";
        mStream->close();
        QUrl url = QUrl::fromLocalFile(mStream->fileName());
        setLoading(false);
        emit loadStreamOk(url.toString());
        return;
    }

    QUrl url(mStreamSegments.first());
    if (!url.isValid())
    {
        qWarning() << "Invalid segment URL:" << mStreamSegments.first();
        setLoading(false);
        emit loadStreamError();
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

    if (!mStream)
    {
        qWarning() << "Stream is not present.";
        setLoading(false);
        emit loadStreamError();
        return;
    }

    if (mStream->write(data) < 0)
    {
        qWarning() << "Failed to save stream into tempfile:" << mStream->fileName();
        setLoading(false);
        emit loadStreamError();
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
    setLoading(false);
    emit loadStreamError();
}

void M3U8Reader::loadStreamSslFailed(QNetworkReply* reply)
{
    mInProgress = nullptr;
    qDebug() << "SSL error, failed to load stream segment:" << reply->request().url();
    setLoading(false);
    emit loadStreamError();
}

}
