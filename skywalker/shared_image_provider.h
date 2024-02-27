// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QHashFunctions>
#include <QMutex>
#include <QQuickImageProvider>
#include <unordered_map>

namespace Skywalker {

// For sharing images via app sharing or Android photo picker
class SharedImageProvider : public QQuickImageProvider
{
public:
    static constexpr char const* SHARED_IMAGE = "sharedimage";
    static SharedImageProvider* getProvider(const QString& name);

    explicit SharedImageProvider(const QString& name);
    ~SharedImageProvider();

    QString addImage(const QImage& image);
    void removeImage(const QString& source);
    QImage getImage(const QString& source);
    void replaceImage(const QString& source, const QImage& image);

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QString getIdFromSource(const QString& source) const;

    QMutex mMutex;
    std::unordered_map<QString, QImage> mImages; // id -> image
    int mNextId = 1;
    QString mName;

    static std::unordered_map<QString, SharedImageProvider*> sProviders; // name -> provider
};

class SharedImageSource
{
public:
    using SharedPtr = std::shared_ptr<SharedImageSource>;

    SharedImageSource(const QString& source, SharedImageProvider* provider);
    ~SharedImageSource();

private:
    QString mSource;
    SharedImageProvider* mProvider;
};

}
