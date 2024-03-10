// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "bookmark.h"
#include "lexicon.h"
#include <atproto/lib/xjson.h>

namespace Skywalker::Bookmark {

QJsonObject Bookmark::toJson() const
{
    QJsonObject json;
    json.insert("$type", Lexicon::COLLECTION_BOOKMARK);
    json.insert("uri", mUri);
    return json;
}

Bookmark::Ptr Bookmark::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    auto bookmark = std::make_unique<Bookmark>();
    bookmark->mUri = xjson.getRequiredString("uri");
    return bookmark;
}

}
