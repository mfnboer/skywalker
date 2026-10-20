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
    using ImageCb = std::function<void(QImage)>;

    static constexpr char const* AVATAR = "cached_avatar";
    static constexpr char const* THUMB_IMG = "cached_thumb_img";
    static CachedImageProvider* getProvider(const QString& name);

    explicit CachedImageProvider(const QString& name);

    QString createImageUrl(const QString& webUrl) const;
    static QString createHttpUrl(const QString& imgUrl);

    void addImage(const QString& id, const QImage& img, const QString& format);
    std::pair<QImage, QString> getImage(const QString& id);

    bool mustLoadImage(const QString& id, const QSize& requestedSize) const;

    // Returns true if it was not yet pending, false if a request is pending already
    // In both cases imageCb will be called when the request is done.
    bool addPendingRequest(const QString& id, const ImageCb& imageCb);
    void finishPendingRequest(const QString& id, QImage img);

    QNetworkAccessManager* getNetwork() const { return mNetwork; }

    // id = image url without scheme
    QQuickImageResponse *requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
    QString createFileName(const QString& baseName, const QString& extension) const;
    std::pair<QString, QString> getBaseNameAndExtension(const QString& id) const;
    QString getFileName(const QString& id) const;
    void cleanupCache();

    QString mName;
    QMutex mCacheMutex;
    QString mCachePath;
    QThreadPool mThreadPool;
    bool mSquareImagesOnly = false;
    QMutex mPendingRequestsMutex;
    std::unordered_map<QString, std::vector<ImageCb>> mPendingRequests; // id -> callback
    int mLoadCount = 0;
    QNetworkAccessManager* mNetwork;

    static std::unordered_map<QString, CachedImageProvider*> sProviders; // name -> provider
};

class CachedImageResponse : public QQuickImageResponse
{
public:
    CachedImageResponse(const QString& providerName, const QString& id, const QSize& requestedSize, QThreadPool* pool);

    QQuickTextureFactory* textureFactory() const override;

private:
    void loadImage();
    void handleDone(QImage img);
    void setImage(QImage img);

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
                         QImage image);
    void run() override;

signals:
    void done(QImage image);

private:
    QString mProviderName;
    QString mId;
    QSize mRequestedSize;
    QImage mImage;
};

}
