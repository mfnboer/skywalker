// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "giphy_image.h"
#include <QJsonObject>
#include <QObject>

namespace Skywalker {

class GiphyGif
{
public:
    using SharedPtr = std::shared_ptr<GiphyGif>;
    using List = std::vector<SharedPtr>;

    static SharedPtr fromJson(const QJsonObject& json);

    const QString& getId() const { return mId; }
    const QString& getTitle() const { return mTitle; }
    const QString getDescription() const { return mDescription.value_or(""); }
    GiphyImage::SharedPtr getOriginal() const { return mOriginal; }
    GiphyImage::SharedPtr getFixedHeight() const { return mFixedHeight; }
    GiphyImage::SharedPtr getOriginalStill() const { return mOriginalStill; }

private:
    QString mId;
    QString mTitle;
    std::optional<QString> mDescription;
    GiphyImage::SharedPtr mOriginal;
    GiphyImage::SharedPtr mFixedHeight;
    GiphyImage::SharedPtr mOriginalStill;
};

}
