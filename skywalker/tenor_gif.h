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
    Q_PROPERTY(QString description MEMBER mDescription CONSTANT FINAL)
    Q_PROPERTY(QString url MEMBER mUrl CONSTANT FINAL)
    Q_PROPERTY(QSize size MEMBER mSize CONSTANT FINAL)
    Q_PROPERTY(QString nanoUrl MEMBER mNanoUrl CONSTANT FINAL)
    Q_PROPERTY(QSize nanoSize MEMBER mNanoSize CONSTANT FINAL)
    Q_PROPERTY(QString previewUrl MEMBER mPreviewUrl CONSTANT FINAL)
    Q_PROPERTY(QSize previewSize MEMBER mPreviewSize CONSTANT FINAL)
    QML_VALUE_TYPE(tenorgif)

public:
    TenorGif(
            const QString& id,
            const QString& description,
            const QString& url, QSize size,
            const QString& nanoUrl, QSize nanoSize,
            const QString& previewUrl, QSize previewSize) :
        mId(id),
        mDescription(description),
        mUrl(url), mSize(size),
        mNanoUrl(nanoUrl), mNanoSize(nanoSize),
        mPreviewUrl(previewUrl), mPreviewSize(previewSize)
    {}

private:
    QString mId;
    QString mDescription;
    QString mUrl;
    QSize mSize;
    QString mNanoUrl;
    QSize mNanoSize;
    QString mPreviewUrl;
    QSize mPreviewSize;
};

using TenorGifList = QList<TenorGif>;

}

Q_DECLARE_METATYPE(Skywalker::TenorGif)
