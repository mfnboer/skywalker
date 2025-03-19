// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "atproto_image_provider.h"
#include "skywalker.h"
#include <QTimer>

namespace Skywalker {

std::unordered_map<QString, ATProtoImageProvider*> ATProtoImageProvider::sProviders;

ATProtoImageProvider* ATProtoImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
        provider = new ATProtoImageProvider(name);

    return provider;
}

ATProtoImageProvider::ATProtoImageProvider(const QString& name) :
    mName(name)
{
}

ATProtoImageProvider::~ATProtoImageProvider()
{
    Q_ASSERT(mImages.empty());
}

QString ATProtoImageProvider::createImageSource(const QString& did, const QString& cid) const
{
    const QString source = QString("image://%1/%2/%3").arg(mName, did, cid);
    return source;
}

QString ATProtoImageProvider::idToSource(const QString& id) const
{
    const QString source = QString("image://%1/%2").arg(mName, id);
    return source;
}

QString ATProtoImageProvider::sourceToId(const QString& source) const
{
    const QString prefix = QString("image://") + DRAFT_IMAGE + "/";
    if (!source.startsWith(prefix))
    {
        qWarning() << "Invalid source:" << source;
        return {};
    }

    return source.mid(prefix.size());
}

void ATProtoImageProvider::addImage(const QString& id, const QImage& img)
{
    const QString source = idToSource(id);

    QMutexLocker locker(&mMutex);
    mImages[source] = img;
}

QImage ATProtoImageProvider::getImage(const QString& source)
{
    QMutexLocker locker(&mMutex);
    auto it = mImages.find(source);
    return it != mImages.end() ? it->second : QImage();
}

void ATProtoImageProvider::clear()
{
    QMutexLocker locker(&mMutex);
    mImages.clear();
}

void ATProtoImageProvider::asyncAddImage(const QString& source, const std::function<void()>& cb)
{
    qDebug() << "Async add image:" << source;
    const QString id = sourceToId(source);

    if (id.isEmpty())
    {
        cb();
        return;
    }

    auto* response = requestImageResponse(id, {});
    connect(response, &QQuickImageResponse::finished, this, [cb, response]{
        cb();
        response->deleteLater();
    });
}

QQuickImageResponse* ATProtoImageProvider::requestImageResponse(const QString& id, const QSize& requestedSize)
{
    auto* response = new ATProtoImageResponse(mName, id, requestedSize);
    return response;
}


ATProtoImageResponse::ATProtoImageResponse(const QString& providerName, const QString& id, const QSize& requestedSize) :
    mNetwork(new QNetworkAccessManager(this)),
    mProviderName(providerName),
    mId(id),
    mRequestedSize(requestedSize)
{
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);

    const auto idParts = id.split('/');

    if (idParts.size() != 2)
    {
        qWarning() << "Invalid id:" << id;
        QTimer::singleShot(0, this, [this]{ handleDone(QImage()); });
        return;
    }

    const QString& did = idParts[0];
    const QString& cid = idParts[1];
    loadImage(did, cid);
}

QQuickTextureFactory* ATProtoImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(mImage);
}

void ATProtoImageResponse::handleDone(QImage img)
{
    if (mRequestedSize.isValid())
        mImage = img.scaled(mRequestedSize);
    else
        mImage = img;

    if (!img.isNull())
    {
        auto* provider = ATProtoImageProvider::getProvider(mProviderName);
        provider->addImage(mId, img);
    }

    emit finished();
}

void ATProtoImageResponse::loadImage(const QString& did, const QString& cid)
{
    qDebug() << "Load image, did:" << did << "cid:" << cid;
    auto xrpc = std::make_unique<Xrpc::Client>(mNetwork);
    xrpc->setUserAgent(Skywalker::getUserAgentString());
    mClient = std::make_unique<ATProto::Client>(std::move(xrpc));

    mClient->getBlob(did, cid,
        [this, did, cid](const QByteArray& bytes, const QString&){
            QImage img;

            if (!img.loadFromData(bytes))
                qWarning() << "Cannot load image from data, did:" << did << "cid:" << cid;

            handleDone(img);
        },
        [this, did, cid](const QString& error, const QString& msg){
            qWarning() << "Failed to load image, did:" << did << "cid:" << cid << error << "-" << msg;
            handleDone(QImage());
        });
}

}
