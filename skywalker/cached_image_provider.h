// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include <QMutex>
#include <QQuickImageProvider>
#include <QThreadPool>

namespace Skywalker {

class CachedImageProvider : public QQuickAsyncImageProvider
{
public:
    static constexpr char const* AVATAR = "cached_avatar";
    static constexpr char const* THUMB_IMG = "cached_thumb_img";
    static CachedImageProvider* getProvider(const QString& name);

    explicit CachedImageProvider(const QString& name);

    QString createImageUrl(const QString& webUrl) const;
    static QString createHttpUrl(const QString& imgUrl);

    void addImage(const QString& id, const QImage& img, const QSize& requestedSize, const QString& format);
    std::pair<QImage, QString> getImage(const QString& id, const QSize& requestedSize);

    bool mustLoadImage(const QString& id, const QSize& requestedSize) const;

    // id = image url without scheme
    QQuickImageResponse *requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
    QString createFileName(const QString& baseName, const QString& extension) const;
    std::pair<QString, QString> getBaseNameAndExtension(const QString& id) const;
    QString getFileName(const QString& id, const QSize& requestedSize) const;
    void cleanupCache();

    QString mName;
    QMutex mCacheMutex;
    QString mCachePath;
    QThreadPool mThreadPool;
    bool mSquareImagesOnly = false;

    static std::unordered_map<QString, CachedImageProvider*> sProviders; // name -> provider
};

class CachedImageResponse : public QQuickImageResponse
{
public:
    CachedImageResponse(const QString& providerName, const QString& id, const QSize& requestedSize, QThreadPool* pool);
    ~CachedImageResponse();

    QQuickTextureFactory* textureFactory() const override;

private:
    void loadImage(const QString& id);
    void handleDone(QImage img, const QString& format, bool cached);

    QString mProviderName;
    QString mId;
    QSize mRequestedSize;
    QImage mImage;
    QThreadPool* mThreadPool;
    ImageReader* mImageReader = nullptr;
};

class CachedImageProcessor : public QObject, public QRunnable
{
    Q_OBJECT

public:
    CachedImageProcessor(const QString& providerName, const QString& id, const QSize& requestedSize,
                         QImage image, const QString& format);
    void run() override;

signals:
    void done(QImage image);

private:
    QString mProviderName;
    QString mId;
    QSize mRequestedSize;
    QImage mImage;
    QString mFormat;
};

}
