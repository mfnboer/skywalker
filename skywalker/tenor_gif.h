// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

// Also used for Giphy
class TenorGif
{
    Q_GADGET
    Q_PROPERTY(QString id READ getId CONSTANT FINAL)
    Q_PROPERTY(QString description READ getDescription CONSTANT FINAL)
    Q_PROPERTY(QString url READ getUrl CONSTANT FINAL)
    Q_PROPERTY(QString smallUrl READ getSmallUrl CONSTANT FINAL)
    Q_PROPERTY(QSize size READ getSize CONSTANT FINAL)
    Q_PROPERTY(QSize smallSize READ getSmallSize CONSTANT FINAL)
    Q_PROPERTY(QString imageUrl READ getImageUrl CONSTANT FINAL)
    Q_PROPERTY(QSize imageSize READ getImageSize CONSTANT FINAL)
    Q_PROPERTY(QSize overviewSize READ getOverviewSize CONSTANT FINAL)
    QML_VALUE_TYPE(tenorgif)

public:
    TenorGif() : mPrivate{std::make_shared<PrivateData>()} {}
    ~TenorGif() = default;
    TenorGif(const TenorGif&) = default;
    TenorGif& operator=(const TenorGif&) = default;

    TenorGif(
            const QString& id,
            const QString& description,
            const QString& searchTerm,
            const QString& url, QSize size,
            const QString& smallUrl, QSize smallSize,
            const QString& imageUrl, QSize imageSize) :
        mPrivate{std::make_shared<PrivateData>(
            id,
            description,
            searchTerm,
            url, size,
            smallUrl, smallSize,
            imageUrl, imageSize,
            smallSize)}
    {}

    Q_INVOKABLE bool isNull() const { return mPrivate->mUrl.isEmpty(); }
    const QString& getId() const { return mPrivate->mId; }
    const QString& getDescription() const { return mPrivate->mDescription; }
    const QString& getUrl() const { return mPrivate->mUrl; }
    const QString& getImageUrl() const { return mPrivate->mImageUrl; }
    const QString& getSearchTerm() const { return mPrivate->mSearchTerm; }
    const QString& getSmallUrl() const { return mPrivate->mSmallUrl; }
    QSize getSize() const { return mPrivate->mSize; }
    QSize getSmallSize() const { return mPrivate->mSmallSize; }
    QSize getImageSize() const { return mPrivate->mImageSize; }
    QSize getOverviewSize() const { return mPrivate->mOverviewSize; }
    void setOverviewSize(QSize size) { mPrivate->mOverviewSize = size; }
    Q_INVOKABLE bool isGiphy() const { return mPrivate->mIsGiphy; }
    void setIsGiphy(bool isGiphy) { mPrivate->mIsGiphy = isGiphy; }

    // Get a URL that is compatible with Bluesky
    Q_INVOKABLE const QString getUrlForPosting() const;

    TenorGif deepCopy() const;

private:
    struct PrivateData
    {
        QString mId;
        QString mDescription;
        QString mSearchTerm;
        QString mUrl;
        QSize mSize;
        QString mSmallUrl;
        QSize mSmallSize;
        QString mImageUrl;
        QSize mImageSize;
        QSize mOverviewSize;
        bool mIsGiphy = false;
    };

    std::shared_ptr<PrivateData> mPrivate;
};

using TenorGifList = QList<TenorGif>;

}

Q_DECLARE_METATYPE(::Skywalker::TenorGif)
