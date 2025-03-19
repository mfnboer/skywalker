// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/client.h>
#include <QHashFunctions>
#include <QMutex>
#include <QQuickImageProvider>
#include <unordered_map>

namespace Skywalker {

class ATProtoImageProvider : public QQuickAsyncImageProvider
{
public:
    static constexpr char const* DRAFT_IMAGE = "draftimage";
    static ATProtoImageProvider* getProvider(const QString& name);

    explicit ATProtoImageProvider(const QString& name);
    ~ATProtoImageProvider();

    QString createImageSource(const QString& did, const QString& cid) const;
    QString idToSource(const QString& id) const;
    QString sourceToId(const QString& source) const;
    void addImage(const QString& id, const QImage& img);
    QImage getImage(const QString& source);
    void clear();

    void asyncAddImage(const QString& source, const std::function<void()>& cb);

    // id = <did>/<cid>
    QQuickImageResponse *requestImageResponse(const QString& id, const QSize& requestedSize) override;

private:
    QString mName;

    QMutex mMutex;
    std::unordered_map<QString, QImage> mImages; // source -> image

    static std::unordered_map<QString, ATProtoImageProvider*> sProviders; // name -> provider
};

class ATProtoImageResponse : public QQuickImageResponse
{
public:
    ATProtoImageResponse(const QString& providerName, const QString& id, const QSize& requestedSize);

    QQuickTextureFactory* textureFactory() const override;

private:
    void loadImage(const QString& did, const QString& cid);
    void handleDone(QImage img);

    QNetworkAccessManager* mNetwork;
    QString mProviderName;
    QString mId;
    QSize mRequestedSize;
    QImage mImage;
    std::unique_ptr<ATProto::Client> mClient;
};

}
