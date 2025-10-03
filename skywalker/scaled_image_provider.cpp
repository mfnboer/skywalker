// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "scaled_image_provider.h"
#include <atproto/lib/time_monitor.h>

namespace Skywalker {

static std::unordered_map<QString, int> mUrlCount;
std::unordered_map<QString, ScaledImageProvider*> ScaledImageProvider::sProviders;

static std::unique_ptr<QNetworkAccessManager> makeNetwork(int networkTransferTimeoutMs, QObject* parent)
{
    auto network = std::make_unique<QNetworkAccessManager>(parent);
    network->setAutoDeleteReplies(true);
    network->setTransferTimeout(networkTransferTimeoutMs);
    return network;
}

ScaledImageThread::ScaledImageThread(QImage image, const QSize &requestedSize) :
    mImage(image),
    mRequestedSize(requestedSize)
{
    setAutoDelete(true);
}

void ScaledImageThread::run()
{
    ATProto::TimeMonitor timeMon("Scaling");
    const auto scaledImage = mImage.scaled(mRequestedSize);
    emit done(scaledImage);
}

ScaledImageRepsonse::ScaledImageRepsonse(const QString& id, const QSize &requestedSize,
                                         QThreadPool& pool, QCache<QString, QImage>* cache,
                                         InProgess& inProgress) :
    mId(id),
    mNetwork(makeNetwork(10000, this)),
    mImageReader(mNetwork.get()),
    mPool(pool),
    mCache(cache),
    mInProgress(inProgress)
{
    Q_ASSERT(cache);
    qDebug() << "Scaled image response:" << mId << requestedSize;
    qDebug() << "Thread:" << QThread::currentThreadId();

    if (mCache->contains(mId))
    {
        qDebug() << "From cache:" << mId;
        scaleImage(*mCache->object(mId), requestedSize);
        return;
    }

    if (!setInProgress())
        return;

    if (++mUrlCount[mId] > 1)
        qDebug() << "AGAIN:" << mUrlCount[mId] << mUrlCount.size() << mId;

    mImageReader.getImageFromWeb(mId,
        [this, requestedSize](QImage image){
            cacheImage(image);
            scaleImage(image, requestedSize);
        },
        [this](const QString& error){
            handleFailed(error);
        }
    );
}

bool ScaledImageRepsonse::setInProgress()
{
    if (mInProgress.contains(mId))
    {
        qDebug() << "Already in progress:" << mId;
        mInProgress[mId].insert(this);
        return false;
    }

    mInProgress[mId] = {};
    return true;
}

void ScaledImageRepsonse::removeFromProgress()
{
    if (!mInProgress.contains(mId))
        return;

    const auto responses = mInProgress[mId];
    qDebug() << "Notify waiting responses:" << responses.size();
    mInProgress.erase(mId);

    // TODO:
    // This is not right. The request size could be different is different responses.
    // The image has been scaled already at this point.
    for (auto* response : responses)
    {
        if (!mImage.isNull())
            response->handleDone(mImage);
        else
            response->handleFailed(mError);
    }
}

void ScaledImageRepsonse::cacheImage(const QImage& image)
{
    if (mId.contains("avatar_thumbnail"))
    {
        mCache->insert(mId, new QImage(image));
        qDebug() << "Cache image:" << mCache->size() << mId << image.sizeInBytes() << "bytes";
    }
}

void ScaledImageRepsonse::scaleImage(QImage image, const QSize &requestedSize)
{
    qDebug() << "Scale from:" << image.size() << "to:" << requestedSize << mId;

    if (requestedSize.isValid())
    {
        auto* runnable = new ScaledImageThread(image, requestedSize);
        connect(runnable, &ScaledImageThread::done,  this, &ScaledImageRepsonse::handleDone);
        mPool.start(runnable);
    }
    else
    {
        handleDone(image);
    }
}

void ScaledImageRepsonse::handleDone(QImage image)
{
    qDebug() << "Done:" << image.size() << mId;
    mImage = image;
    emit finished();

    removeFromProgress();
}

void ScaledImageRepsonse::handleFailed(QString error)
{
    qDebug() << "Failed:" << error << mId;
    mError = error;
    emit finished();

    removeFromProgress();
}

QString ScaledImageRepsonse::errorString() const
{
    return mError;
}

QQuickTextureFactory* ScaledImageRepsonse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(mImage);
}

ScaledImageProvider* ScaledImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
        provider = new ScaledImageProvider(name);

    return provider;
}

ScaledImageProvider::ScaledImageProvider(const QString& name) :
    mName(name),
    mCache(50)
{
}

QQuickImageResponse* ScaledImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    auto* response = new ScaledImageRepsonse(id, requestedSize, mThreadPool, &mCache, mInprogress);
    return response;
}

}
