// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

void jsonInsert(QJsonObject& json, const QString& key, const QSize& sz)
{
    json.insert(key, QJsonObject{ {"width", sz.width() }, {"height", sz.height() } });
}

QSize getRequiredSize(const QJsonObject& json, const QString& key)
{
    const ATProto::XJsonObject& xjson(json);
    const QJsonObject sizeJson = xjson.getRequiredJsonObject(key);
    const ATProto::XJsonObject& sizeXJson(sizeJson);

    QSize sz;
    sz.setWidth(sizeXJson.getRequiredInt("width"));
    sz.setHeight(sizeXJson.getRequiredInt("height"));

    return sz;
}

QJsonObject TenorGif::toJson() const
{
    QJsonObject json;
    json.insert("id", mId);
    json.insert("description", mDescription);
    json.insert("searchTerm", mSearchTerm);
    json.insert("url", mUrl);
    json.insert("smallUrl", mSmallUrl);
    jsonInsert(json, "smallSize", mSmallSize);
    json.insert("imageUrl", mImageUrl);
    jsonInsert(json, "imageSize", mImageSize);

    return json;
}

TenorGif TenorGif::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject& xjson(json);

    try {
        TenorGif gif{
            xjson.getRequiredString("id"),
            xjson.getRequiredString("description"),
            xjson.getRequiredString("searchTerm"),
            xjson.getRequiredString("url"),
            xjson.getRequiredString("smallUrl"),
            getRequiredSize(json, "smallSize"),
            xjson.getRequiredString("imageUrl"),
            getRequiredSize(json, "imageSize")
        };

        return gif;
    } catch (ATProto::InvalidJsonException& e) {
        qWarning() << e.msg();
    }

    return {};
}

QDataStream& operator<<(QDataStream& out, const TenorGif& gif)
{
    QJsonDocument json(gif.toJson());
    return out << json.toJson(QJsonDocument::Compact);
}

QDataStream& operator>>(QDataStream& in, TenorGif& gif)
{
    QByteArray data;
    in >> data;
    const auto json = QJsonDocument::fromJson(data);
    gif = TenorGif::fromJson(json.object());
    return in;
}

}
