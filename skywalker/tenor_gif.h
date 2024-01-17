// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TenorGif
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER mId CONSTANT FINAL)
    Q_PROPERTY(QString description READ getDescription CONSTANT FINAL)
    Q_PROPERTY(QString url MEMBER mUrl CONSTANT FINAL)
    Q_PROPERTY(QSize size MEMBER mSize CONSTANT FINAL)
    Q_PROPERTY(QString smallUrl MEMBER mSmallUrl CONSTANT FINAL)
    Q_PROPERTY(QSize smallSize MEMBER mSmallSize CONSTANT FINAL)
    Q_PROPERTY(QString imageUrl MEMBER mImageUrl CONSTANT FINAL)
    Q_PROPERTY(QSize imageSize MEMBER mImageSize CONSTANT FINAL)
    Q_PROPERTY(QSize overviewSize READ getOverviewSize CONSTANT FINAL)
    QML_VALUE_TYPE(tenorgif)

public:
    TenorGif() = default;
    TenorGif(
            const QString& id,
            const QString& description,
            const QString& searchTerm,
            const QString& url, QSize size,
            const QString& smallUrl, QSize smallSize,
            const QString& imageUrl, QSize imageSize) :
        mId(id),
        mDescription(description),
        mSearchTerm(searchTerm),
        mUrl(url), mSize(size),
        mSmallUrl(smallUrl), mSmallSize(smallSize),
        mImageUrl(imageUrl), mImageSize(imageSize),
        mOverviewSize(mSmallSize)
    {}

    const QString& getId() const { return mId; }
    const QString& getDescription() const { return mDescription; }
    const QString& getSearchTerm() const { return mSearchTerm; }
    QSize getOverviewSize() const { return mOverviewSize; }
    void setOverviewSize(QSize size) { mOverviewSize = size; }

private:
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

using TenorGifList = QList<TenorGif>;

}

Q_DECLARE_METATYPE(Skywalker::TenorGif)
