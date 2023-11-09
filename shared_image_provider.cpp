// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "shared_image_provider.h"

namespace Skywalker {

std::map<QString, SharedImageProvider*> SharedImageProvider::sProviders;

SharedImageProvider* SharedImageProvider::getProvider(const QString& name)
{
    auto& provider = sProviders[name];

    if (!provider)
        provider = new SharedImageProvider;

    return provider;
}

SharedImageProvider::SharedImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image)
{
}

QString SharedImageProvider::addImage(const QImage& image)
{
    QString id = QString("SharedImg_%1").arg(mNextId++);
    mImages[id] = image;
    return id;
}

void SharedImageProvider::removeImage(const QString& id)
{
    mImages.erase(id);
}

QImage SharedImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
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

}
