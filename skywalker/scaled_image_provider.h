// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "image_reader.h"
#include <QCache>
#include <QQuickImageProvider>
#include <QThreadPool>
#include <unordered_map>
#include <unordered_set>

namespace Skywalker {

class ScaledImageThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ScaledImageThread(QImage image, const QSize &requestedSize);

    void run() override;

signals:
    void done(QImage image);

private:
    QImage mImage;
    QSize mRequestedSize;
};

class ScaledImageRepsonse : public QQuickImageResponse
{
public:
    using InProgess = std::unordered_map<QString, std::unordered_set<ScaledImageRepsonse*>>;

    ScaledImageRepsonse(const QString& id, const QSize &requestedSize, QThreadPool& pool,
                        QCache<QString, QImage>* cache, InProgess& inProgress);

    void handleDone(QImage image);
    void handleFailed(QString error);

    QString errorString() const override;
    QQuickTextureFactory *textureFactory() const override;

private:
    bool setInProgress();
    void removeFromProgress();
    void cacheImage(const QImage& image);
    void scaleImage(QImage image, const QSize &requestedSize);

    QString mId;
    std::unique_ptr<QNetworkAccessManager> mNetwork;
    ImageReader mImageReader;
    QImage mImage;
    QString mError;
    QThreadPool& mPool;
    QCache<QString, QImage>* mCache;
    InProgess& mInProgress;
};

class ScaledImageProvider : public QQuickAsyncImageProvider
{
public:
    static constexpr char const* SCALED_IMAGE = "scaledimage";
    static ScaledImageProvider* getProvider(const QString& name);

    explicit ScaledImageProvider(const QString& name);

    // id = image URL
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
    static std::unordered_map<QString, ScaledImageProvider*> sProviders; // name -> provider

    QString mName;
    QThreadPool mThreadPool;
    QCache<QString, QImage> mCache; // id -> image
    ScaledImageRepsonse::InProgess mInprogress;
};

}
