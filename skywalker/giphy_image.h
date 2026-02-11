// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>
#include <QObject>
#include <QSize>

namespace Skywalker {

class GiphyImage
{
public:
    using SharedPtr = std::shared_ptr<GiphyImage>;

    static SharedPtr fromJson(const QJsonObject& json);

    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }
    QSize getSize() const { return QSize(mWidth, mHeight); }
    const QString& getUrl() const { return mUrl; }

private:
    int mWidth;
    int mHeight;
    QString mUrl;

};

}
