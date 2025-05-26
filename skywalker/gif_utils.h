// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

// Utils for handling different GIF links send by various apps.
class GifUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GifUtils(QObject* parent = nullptr);

    Q_INVOKABLE bool isTenorLink(const QString& link) const;
    Q_INVOKABLE bool isGiphyLink(const QString& link) const;
    Q_INVOKABLE bool isGif(const QString& link) const;
    Q_INVOKABLE QString getGifUrl(const QString& link) const;
    Q_INVOKABLE QSize gitGifSize(const QString& link) const;

private:
    QString getGiphyGifUrl(const QString& link) const;
    QString getGiphyMediaUrl(const QString& link) const;
    QString getTenorViewGif(const QString& link) const;
    QString getTenorMediaGif(const QString& link) const;
    QString convertGrayskyToTenor(const QString& link) const;
};

}
