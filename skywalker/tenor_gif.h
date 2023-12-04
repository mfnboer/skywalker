// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class TenorGif : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER mId CONSTANT FINAL)
    Q_PROPERTY(QString description MEMBER mDescription CONSTANT FINAL)
    Q_PROPERTY(QString gifUrl MEMBER mGifUrl CONSTANT FINAL)
    Q_PROPERTY(QSize gifSize MEMBER mGifSize CONSTANT FINAL)
    Q_PROPERTY(QString tinyGifUrl MEMBER mTinyGifUrl CONSTANT FINAL)
    Q_PROPERTY(QSize tinyGifSize MEMBER mTinyGifSize CONSTANT FINAL)
    Q_PROPERTY(QString previewUrl MEMBER mPreviewUrl CONSTANT FINAL)
    Q_PROPERTY(QSize previewSize MEMBER mPreviewSize CONSTANT FINAL)
    QML_ELEMENT

public:
    TenorGif(
            const QString& id,
            const QString& description,
            const QString& gifUrl, QSize gifSize,
            const QString& tinyGifUrl, QSize tinyGifSize,
            const QString& previewUrl, QSize previewSize,
            QObject* parent = nullptr) :
        QObject(parent),
        mId(id),
        mDescription(description),
        mGifUrl(gifUrl), mGifSize(gifSize),
        mTinyGifUrl(tinyGifUrl), mTinyGifSize(tinyGifSize),
        mPreviewUrl(previewUrl), mPreviewSize(previewSize)
    {}

private:
    QString mId;
    QString mDescription;
    QString mGifUrl;
    QSize mGifSize;
    QString mTinyGifUrl;
    QSize mTinyGifSize;
    QString mPreviewUrl;
    QSize mPreviewSize;
};

using TenorGifList = QList<TenorGif>;

}
