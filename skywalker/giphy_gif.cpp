// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "giphy_gif.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

GiphyGif::SharedPtr GiphyGif::fromJson(const QJsonObject& json)
{
    auto gif = std::make_shared<GiphyGif>();
    ATProto::XJsonObject xjson(json);
    gif->mId = xjson.getRequiredString("id");
    gif->mTitle = xjson.getRequiredString("title");
    gif->mDescription = xjson.getOptionalString("alt_text");

    const auto imagesJson = xjson.getRequiredJsonObject("images");
    ATProto::XJsonObject imagesXjson(imagesJson);

    gif->mOriginal = imagesXjson.getRequiredObject<GiphyImage>("original");
    gif->mFixedHeight = imagesXjson.getRequiredObject<GiphyImage>("fixed_height");
    gif->mOriginalStill = imagesXjson.getRequiredObject<GiphyImage>("original_still");

    return gif;
}

}
