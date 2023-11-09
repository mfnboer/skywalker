// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QQuickImageProvider>

namespace Skywalker {

// Provides image shared via copy/paste or app sharing
class SharedImageProvider : public QQuickImageProvider
{
public:
    static constexpr char const* SHARED_IMAGE = "sharedimage";
    static SharedImageProvider* getProvider(const QString& name);

    SharedImageProvider();

    QString addImage(const QImage& image);
    void removeImage(const QString& id);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    std::map<QString, QImage> mImages; // id -> image
    int mNextId = 1;

    static std::map<QString, SharedImageProvider*> sProviders; // name -> provider
};

}
