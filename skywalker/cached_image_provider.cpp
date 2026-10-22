// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "cached_image_provider.h"
#include "display_utils.h"
#include "file_utils.h"
#include "network_holder.h"
#include <atproto/lib/at_regex.h>
#include <QCryptographicHash>
#include <QDir>
#include <QImageReader>
#include <QTimer>

namespace Skywalker {

static constexpr int MAX_CACHE_SIZE = 512;
static constexpr qint64 MAX_CACHE_BYTES = 128 * 1024 * 1024;

static bool mustResize(const QSize& requestedSize)
{
    return requestedSize.isValid() && !requestedSize.isNull();
}

std::unordered_map<QString, CachedImageProvider*> CachedImageProvider::sProviders;

CachedImageProvider* CachedImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
    {
        provider = new CachedImageProvider(name);

        if (name == AVATAR)
            provider->mSquareImagesOnly = true;
    }

    return provider;
}

CachedImageProvider::CachedImageProvider(const QString& name) :
    mName(name),
    mCachePath(FileUtils::getCachePath(mName)),
    mImageReader(NetworkHolder::network(), this)
{   
    qDebug() << "Image provider:" << mName << "cache:" << mCachePath;
}

QString CachedImageProvider::createImageUrl(const QString& webUrl) const
{
    if (webUrl.isEmpty())
        return {};

    const QUrl url(webUrl);

    if (!url.isValid())
    {
        qWarning() << "Invalid url:" << webUrl;
        return {};
    }

    const QString noScheme = url.toString(QUrl::RemoveScheme).sliced(2); // slice off "//"
    const QString imageUrl = QString("image://%1/%2").arg(mName, noScheme);
    return imageUrl;
}

QString CachedImageProvider::createHttpUrl(const QString& imgUrl)
{
    static const QRegularExpression sourceRE(R"(image:\/\/[^\/]+\/(.+))");

    auto match = sourceRE.match(imgUrl);

    if (!match.hasMatch())
    {
        qWarning() << "Invalid image url:" << imgUrl;
        return imgUrl;
    }

    const QString httpUrl = "https://" + match.captured(1);
    qDebug() << "Converted:" << imgUrl << "to:" << httpUrl;
    return httpUrl;
}

void CachedImageProvider::addImage(const QString& id, const QImage& img, const QString& format)
{
    const QString fileName = getFileName(id);

    if (fileName.isEmpty())
        return;

    if (!img.save(fileName, !format.isEmpty() ? format.toUtf8() : nullptr))
        qWarning() << "Failed to save image:" << fileName;
    else
        qDebug() << "Cached image:" << fileName;

    cleanupCache();
}

std::pair<QImage, QString> CachedImageProvider::getImage(const QString& id)
{
    const QString fileName = getFileName(id);

    if (fileName.isEmpty())
        return {};

    ++mLoadCount;
    qDebug() << mName << "Load count:" << mLoadCount << id;

    if (!QFile::exists(fileName))
    {
        qDebug() << "Image not in cache:" << id << "provider:" << mName;
        return {};
    }

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    QString format = reader.format();

    if (img.isNull())
    {
        qWarning() << "Failed to read:" << fileName << "error:" << reader.errorString() << "format:" << format;
    }
    else
    {
        FileUtils::updateModificationTime(fileName);
        qDebug() << "Image loaded from cache:" << id << "file:" << fileName << "format:" << format << "provider:" << mName;
    }

    return { img, format };
}

QString CachedImageProvider::createFileName(const QString& baseName, const QString& extension) const
{
    const QString fileName = extension.isEmpty() ?
        QString("%1/%2").arg(mCachePath, baseName) :
        QString("%1/%2.%3").arg(mCachePath, baseName, extension);
    return fileName;
}
std::pair<QString, QString> CachedImageProvider::getBaseNameAndExtension(const QString& id) const
{
    if (id.isEmpty())
    {
        qWarning() << "Invalid id:" << id << "provider:" << mName;
        return {};
    }

    const QString baseName = QCryptographicHash::hash(id.toUtf8(), QCryptographicHash::Sha256).
                             toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    const QString extension = QFileInfo(id).suffix();

    return { baseName, extension };
}

QString CachedImageProvider::getFileName(const QString& id) const
{
    const auto [baseName, extension] = getBaseNameAndExtension(id);

    if (baseName.isEmpty())
        return {};

    return createFileName(baseName, extension);
}

bool CachedImageProvider::mustLoadImage(const QString& id,const QSize& requestedSize) const
{
    // If the requested size is invalid, then no resizing is requested.
    if (!mustResize(requestedSize))
        return true;

    if (requestedSize.isEmpty())
    {
        qDebug() << "Empty size:" << id << "size:" << requestedSize << "provider:" << mName;
        return false;
    }

    if (mSquareImagesOnly && requestedSize.width() != requestedSize.height())
    {
        qDebug() << "Image is not square:" << id << "size:" << requestedSize << "provider:" << mName;
        return false;
    }

    return true;
}

QQuickImageResponse* CachedImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    // HACK
    // The sourceSize on an image is set in device pixels (need images fetched by Qt itself over
    // HTTP). For an image provide Qt multiplies the sourceSize by the DPR to get the
    // requestedSize. This causes the DPR to apply twice. So we divide by DPR again.

    const auto size = requestedSize / DisplayUtils::getDevicePixelRatio();
    auto* response = new CachedImageResponse(mName, id, size, &mThreadPool);
    response->loadImage();
    return response;
}

void CachedImageProvider::cleanupCache()
{
    qDebug() << "Cleanup cache:" << mName;

    if (!mCacheMutex.tryLock())
    {
        qDebug() << "Cleanup already running";
        return;
    }

    QDir cacheDir(mCachePath, "", QDir::Time | QDir::Reversed, QDir::Files);
    const QFileInfoList fileInfos = cacheDir.entryInfoList();
    qint64 cacheBytes = 0;

    for (const auto& info : fileInfos)
        cacheBytes += info.size();

    qDebug() << "Cache size:" << fileInfos.size() << "bytes:" << cacheBytes;

    if (fileInfos.size() <= MAX_CACHE_SIZE && cacheBytes <= MAX_CACHE_BYTES)
    {
        mCacheMutex.unlock();
        return;
    }

    const int cleanCount = fileInfos.size() - MAX_CACHE_SIZE;

    for (int i = 0; i < cleanCount || cacheBytes > MAX_CACHE_BYTES; ++i)
    {
        if (i >= fileInfos.size())
        {
            qWarning() << "No more files left";
            break;
        }

        const QString fileName = fileInfos[i].fileName();
        const qint64 fileBytes = fileInfos[i].size();
        qDebug() << "Remove:" << fileName << "bytes:" << fileBytes << "cacheBytes:" << cacheBytes;

        if (!cacheDir.remove(fileName))
            qWarning() << "Failed to remove:" << fileName;
        else
            cacheBytes -= fileBytes;
    }

    mCacheMutex.unlock();
}

bool CachedImageProvider::addPendingRequest(const QString& id, const ImageCb& imageCb)
{
    qDebug() << "Add pending request:" << id;
    QMutexLocker locker(&mPendingRequestsMutex);

    const bool alreadyPending = mPendingRequests.contains(id);
    mPendingRequests[id].push_back(imageCb);
    return !alreadyPending;
}

void CachedImageProvider::finishPendingRequest(const QString& id, QImage img)
{
    std::vector<ImageCb> callbacks;

    {
        QMutexLocker locker(&mPendingRequestsMutex);
        callbacks = std::move(mPendingRequests[id]);
        mPendingRequests.erase(id);
    }

    qDebug() << "Finish pending request:" << id << "count:" << callbacks.size();

    for (const auto& cb : callbacks)
        cb(img);
}

CachedImageResponse::CachedImageResponse(const QString& providerName, const QString& id,
                                         const QSize& requestedSize, QThreadPool* pool) :
    mProviderName(providerName),
    mId(id),
    mRequestedSize(requestedSize),
    mThreadPool(pool)
{
    Q_ASSERT(mThreadPool);
}

QQuickTextureFactory* CachedImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(mImage);
}

void CachedImageResponse::loadImage()
{
    auto* provider = CachedImageProvider::getProvider(mProviderName);

    if (!provider->mustLoadImage(mId, mRequestedSize))
    {
        handleDone(QImage());
        return;
    }

    const auto [image, format] = provider->getImage(mId);

    if (!image.isNull())
    {
        handleDone(image);
        return;
    }

    if (!provider->addPendingRequest(mId, [this](QImage img){ handleDone(img); }))
        return;

    auto* imageReader = provider->getImageReader();
    QMetaObject::invokeMethod(imageReader, [self=QPointer<ImageReaderResponse>(this), url=mId, imageReader]{
        if (self)
            imageReader->getAsyncImageFromWeb(url, self);
    });
}

void CachedImageResponse::onError(QString error)
{
    qWarning() << "Failed to load image:" << mId << "provider:" << mProviderName << "error:" << error;
    auto* provider = CachedImageProvider::getProvider(mProviderName);
    provider->finishPendingRequest(mId, QImage());
}

void CachedImageResponse::onImage(QImage image, QString format)
{
    auto* provider = CachedImageProvider::getProvider(mProviderName);

    // Save the unscaled image
    if (!image.isNull())
        provider->addImage(mId, image, format);

    provider->finishPendingRequest(mId, image);
}

void CachedImageResponse::handleDone(QImage img)
{
    if (img.isNull() || !mustResize(mRequestedSize))
    {
        setImage(img);
        emit finished();
        return;
    }

    auto* runnable = new CachedImageProcessor(mProviderName, mId, mRequestedSize, img);
    connect(runnable, &CachedImageProcessor::done, this,
            [this](QImage image){
                setImage(image);
                emit finished();
            });
    mThreadPool->start(runnable);
}

void CachedImageResponse::setImage(QImage img)
{
    mImage = img;
    mImage.setDevicePixelRatio(DisplayUtils::getDevicePixelRatio());
}


CachedImageProcessor::CachedImageProcessor(const QString& providerName, const QString& id, const QSize& requestedSize,
                                           QImage image) :
    mProviderName(providerName),
    mId(id),
    mRequestedSize(requestedSize),
    mImage(image)
{
}

void CachedImageProcessor::run()
{
    // Expanding ratio as the image will be cropped if it exceeds the requested size
    const QImage img =  !mustResize(mRequestedSize) ? mImage.scaled(mRequestedSize, Qt::KeepAspectRatioByExpanding) : mImage;
    emit done(img);
}

}
