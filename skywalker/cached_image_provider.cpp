// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "cached_image_provider.h"
#include "file_utils.h"
#include <QDir>
#include <QImageReader>
#include <QTimer>

namespace Skywalker {

static constexpr int MAX_CACHE_SIZE = 512;

std::unordered_map<QString, CachedImageProvider*> CachedImageProvider::sProviders;

CachedImageProvider* CachedImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
        provider = new CachedImageProvider(name);

    return provider;
}

CachedImageProvider::CachedImageProvider(const QString& name) :
    mName(name),
    mCachePath(FileUtils::getCachePath(mName))
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
    qDebug() << "Convert:" << webUrl << "to:" << imageUrl;
    return imageUrl;
}

void CachedImageProvider::addImage(const QString& id, const QImage& img, const QSize& requestedSize, const QString& format)
{
    const QString fileName = getFileName(id, requestedSize);

    if (fileName.isEmpty())
        return;

    if (!img.save(fileName, format.toUtf8()))
        qWarning() << "Failed to save image:" << fileName;
    else
        qDebug() << "Cached image:" << fileName;

    cleanupCache();
}

std::pair<QImage, QString> CachedImageProvider::getImage(const QString& id, const QSize& requestedSize)
{
    const QString fileName = getFileName(id, requestedSize);

    if (fileName.isEmpty())
        return {};

    if (!QFile::exists(fileName))
    {
        qDebug() << "Image not in cache:" << id << "size:" << requestedSize << "provider:" << mName;
        return {};
    }

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage img = reader.read();

    if (img.isNull())
        qWarning() << "Failed to read:" << fileName;
    else
        FileUtils::updateModificationTime(fileName);

    const QString format = reader.format();
    return { img, format };
}

QString CachedImageProvider::createFileName(const QString& baseName) const
{
    const QString fileName = QString("%1/%2").arg(mCachePath, baseName);
    return fileName;
}
QString CachedImageProvider::getBaseName(const QString& id) const
{
    const auto idParts = id.split('/');
    const QString& baseName = idParts.back();

    if (baseName.isEmpty())
        qWarning() << "Invalid id:" << id << "provider:" << mName;

    return baseName;
}

QString CachedImageProvider::getFileName(const QString& id, const QSize& requestedSize) const
{
    const QString baseName = getBaseName(id);

    if (baseName.isEmpty())
        return {};

    if (!requestedSize.isValid())
        return createFileName(baseName);

    const int width = requestedSize.isValid() ? requestedSize.width() : 0;
    const int height = requestedSize.isValid() ? requestedSize.height() : 0;
    const QString newBaseName = QString("%1-%2_%3").arg(baseName).arg(width).arg(height);
    return createFileName(newBaseName);
}

QQuickImageResponse* CachedImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    auto* response = new CachedImageResponse(mName, id, requestedSize, &mThreadPool);
    return response;
}

void CachedImageProvider::cleanupCache()
{
    qDebug() << "Cleanup cache";

    if (!mCacheMutex.tryLock())
    {
        qDebug() << "Cleanup already running";
        return;
    }

    QDir cacheDir(mCachePath, "", QDir::Time | QDir::Reversed, QDir::Files);
    const QStringList fileNames = cacheDir.entryList();
    qDebug() << "Cache size:" << fileNames.size();

    if (fileNames.size() <= MAX_CACHE_SIZE)
    {
        mCacheMutex.unlock();
        return;
    }

    const int cleanCount = fileNames.size() - MAX_CACHE_SIZE;

    for (int i = 0; i < cleanCount; ++i)
    {
        qDebug() << "Remove:" << fileNames[i];

        if (!cacheDir.remove(fileNames[i]))
            qWarning() << "Failed to remove:" << fileNames[i];
    }

    mCacheMutex.unlock();
}

CachedImageResponse::CachedImageResponse(const QString& providerName, const QString& id,
                                         const QSize& requestedSize, QThreadPool* pool) :
    mProviderName(providerName),
    mId(id),
    mRequestedSize(requestedSize),
    mThreadPool(pool)
{
    Q_ASSERT(mThreadPool);
    loadImage(id);
}

CachedImageResponse::~CachedImageResponse()
{
}

QQuickTextureFactory* CachedImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(mImage);
}

void CachedImageResponse::loadImage(const QString& id)
{   
    qDebug() << "Load image:" << id << "size:" << mRequestedSize << "provider:" << mProviderName;

    if (mRequestedSize.isEmpty())
    {
        handleDone(QImage(), {}, false);
        return;
    }

    if (mRequestedSize.width() != mRequestedSize.height())
    {
        qDebug() << "Image is not square:" << id << "size:" << mRequestedSize << "provider:" << mProviderName;
        handleDone(QImage(), {}, false);
        return;
    }

    auto* provider = CachedImageProvider::getProvider(mProviderName);

    const auto [image, format] = provider->getImage(id, mRequestedSize);

    if (!image.isNull())
    {
        qDebug() << "Got image from cache:" << id << "provider:" << mProviderName << "format:" << format;
        handleDone(image, format, true);
        return;
    }

    mImageReader = new ImageReader(this);

    const bool success = mImageReader->getImageFromWeb(id,
        [this](QImage image, const QString& format){ handleDone(image, format, false); },
        [this](const QString& error){
            qWarning() << "Failed to load image:" << mId << "provider:" << mProviderName << "error:" << error;
            handleDone(QImage(), {}, false);
        });

    if (!success)
    {
        qWarning() << "Invalid id:" << id << "provider:" << mProviderName;
        handleDone(QImage(), {}, false);
        return;
    }
}

void CachedImageResponse::handleDone(QImage img, const QString& format, bool cached)
{
    // NOTE: cached images have the requested size
    if (cached || img.isNull() || !mRequestedSize.isValid())
    {
        mImage = img;
        emit finished();
        return;
    }

    auto* runnable = new CachedImageProcessor(mProviderName, mId, mRequestedSize, img, format);
    connect(runnable, &CachedImageProcessor::done, this,
            [this](QImage image){
                mImage = image;
                emit finished();
            });
    mThreadPool->start(runnable);
}


CachedImageProcessor::CachedImageProcessor(const QString& providerName, const QString& id, const QSize& requestedSize,
                                           QImage image, const QString& format) :
    mProviderName(providerName),
    mId(id),
    mRequestedSize(requestedSize),
    mImage(image),
    mFormat(format)
{
}

void CachedImageProcessor::run()
{
    // NOTE: cached images have the requested size
    const QImage img =  mRequestedSize.isValid() ? mImage.scaled(mRequestedSize) : mImage;

    if (!img.isNull())
    {
        auto* provider = CachedImageProvider::getProvider(mProviderName);
        provider->addImage(mId, img, mRequestedSize, mFormat);
    }

    emit done(img);
}

}
