// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "giphy_image.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

GiphyImage::SharedPtr GiphyImage::fromJson(const QJsonObject& json)
{
    auto image = std::make_shared<GiphyImage>();
    ATProto::XJsonObject xjson(json);
    image->mWidth = xjson.getRequiredString("width").toInt();
    image->mHeight = xjson.getRequiredString("height").toInt();
    image->mUrl = xjson.getRequiredString("url");
    return image;
}

}
