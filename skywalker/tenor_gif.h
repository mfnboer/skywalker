// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

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
    TenorGif() : mData{std::make_shared<Data>()} {}
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
        mData{std::make_shared<Data>(
            id,
            description,
            searchTerm,
            url, size,
            smallUrl, smallSize,
            imageUrl, imageSize,
            smallSize)}
    {}

    Q_INVOKABLE bool isNull() const { return mData->mUrl.isEmpty(); }
    const QString& getId() const { return mData->mId; }
    const QString& getDescription() const { return mData->mDescription; }
    const QString& getUrl() const { return mData->mUrl; }
    const QString& getImageUrl() const { return mData->mImageUrl; }
    const QString& getSearchTerm() const { return mData->mSearchTerm; }
    const QString& getSmallUrl() const { return mData->mSmallUrl; }
    QSize getSize() const { return mData->mSize; }
    QSize getSmallSize() const { return mData->mSmallSize; }
    QSize getImageSize() const { return mData->mImageSize; }
    QSize getOverviewSize() const { return mData->mOverviewSize; }
    void setOverviewSize(QSize size) { mData->mOverviewSize = size; }

    // Get a URL that is compatible with Bluesky
    Q_INVOKABLE const QString getUrlForPosting() const;

private:
    struct Data
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
    };

    std::shared_ptr<Data> mData;
};

using TenorGifList = QList<TenorGif>;

}

Q_DECLARE_METATYPE(::Skywalker::TenorGif)
