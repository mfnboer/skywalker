// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "shared_image_provider.h"
#include <QRegularExpression>

namespace Skywalker {

std::map<QString, SharedImageProvider*> SharedImageProvider::sProviders;

SharedImageProvider* SharedImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
        provider = new SharedImageProvider(name);

    return provider;
}

SharedImageProvider::SharedImageProvider(const QString& name) :
    QQuickImageProvider(QQuickImageProvider::Image),
    mName(name)
{
}

SharedImageProvider::~SharedImageProvider()
{
    Q_ASSERT(mImages.empty());
}

QString SharedImageProvider::getIdFromSource(const QString& source) const
{
    static const QRegularExpression sourceRE(R"(image://.+/(.+))");

    auto match = sourceRE.match(source);
    Q_ASSERT(match.hasMatch());

    if (!match.hasMatch())
    {
        qWarning() << "Invalid source:" << source;
        return {};
    }

    const QString id = match.captured(1);
    return id;
}

QString SharedImageProvider::addImage(const QImage& image)
{
    QString id = QString("SharedImg_%1").arg(mNextId++);
    mImages[id] = image;
    QString source = QString("image://%1/%2").arg(mName, id);
    qDebug() << "Added img source:" << source << "total:" << mImages.size();
    return source;
}

void SharedImageProvider::removeImage(const QString& source)
{
    const QString id = getIdFromSource(source);

    if (id.isEmpty())
        return;

    mImages.erase(id);
    qDebug() << "Removed source:" << source << "id:" << id << "total:" << mImages.size();
}

QImage SharedImageProvider::getImage(const QString& source)
{
    const QString id = getIdFromSource(source);

    if (id.isEmpty())
        return {};

    const auto it = mImages.find(id);

    if (it == mImages.end())
    {
        qWarning() << "Image not found:" << source;
        return {};
    }

    return it->second;
}

void SharedImageProvider::replaceImage(const QString& source, const QImage& image)
{
    const QString id = getIdFromSource(source);

    if (id.isEmpty())
        return;

    const auto it = mImages.find(id);

    if (it == mImages.end())
    {
        qWarning() << "Image not found:" << source;
        return;
    }

    mImages[id] = image;
    qDebug() << "Replaced image for source:" << source << "id:" << id;
}

QImage SharedImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
    // TODO: need lock? Can be called by multiple threads
    const auto it = mImages.find(id);

    if (it == mImages.end())
        return {};

    QImage img = it->second;

    if (size)
        *size = img.size();

    if (requestedSize.isValid())
    {
        qDebug() << "Scale img size:" << img.size() << "to:" << requestedSize;
        img = img.scaled(requestedSize);
    }

    return img;
}

SharedImageSource::SharedImageSource(const QString& source, SharedImageProvider* provider) :
    mSource(source),
    mProvider(provider)
{
    Q_ASSERT(mProvider);
    Q_ASSERT(source.startsWith("image://"));
}

SharedImageSource::~SharedImageSource()
{
    mProvider->removeImage(mSource);
}

}
