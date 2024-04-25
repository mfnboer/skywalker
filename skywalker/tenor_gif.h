// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QDataStream>
#include <QJsonObject>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TenorGif
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER mId CONSTANT FINAL)
    Q_PROPERTY(QString description READ getDescription CONSTANT FINAL)
    Q_PROPERTY(QString url MEMBER mUrl CONSTANT FINAL)
    Q_PROPERTY(QString smallUrl MEMBER mSmallUrl CONSTANT FINAL)
    Q_PROPERTY(QSize size MEMBER mSize CONSTANT FINAL)
    Q_PROPERTY(QSize smallSize MEMBER mSmallSize CONSTANT FINAL)
    Q_PROPERTY(QString imageUrl MEMBER mImageUrl CONSTANT FINAL)
    Q_PROPERTY(QSize imageSize MEMBER mImageSize CONSTANT FINAL)
    Q_PROPERTY(QSize overviewSize READ getOverviewSize CONSTANT FINAL)
    QML_VALUE_TYPE(tenorgif)

public:
    TenorGif() = default;
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
        mId(id),
        mDescription(description),
        mSearchTerm(searchTerm),
        mUrl(url), mSize(size),
        mSmallUrl(smallUrl), mSmallSize(smallSize),
        mImageUrl(imageUrl), mImageSize(imageSize),
        mOverviewSize(mSmallSize)
    {
        fixSize();
    }

    Q_INVOKABLE bool isNull() const { return mUrl.isEmpty(); }
    const QString& getId() const { return mId; }
    const QString& getDescription() const { return mDescription; }
    const QString& getUrl() const { return mUrl; }
    const QString& getImageUrl() const { return mImageUrl; }
    const QString& getSearchTerm() const { return mSearchTerm; }
    const QString& getSmallUrl() const { return mSmallUrl; }
    QSize getSize() const { return mSize; }
    QSize getSmallSize() const { return mSmallSize; }
    QSize getOverviewSize() const { return mOverviewSize; }
    void setOverviewSize(QSize size) { mOverviewSize = size; }

    // Get a URL that is compatible with Bluesky
    Q_INVOKABLE const QString getUrlForPosting() const;

    QJsonObject toJson() const;
    static TenorGif fromJson(const QJsonObject& json);

private:
    void fixSize();

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

Q_DECLARE_METATYPE(::Skywalker::TenorGif)
