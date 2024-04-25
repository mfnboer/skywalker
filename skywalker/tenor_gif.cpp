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

    if (sz.isEmpty())
        throw ATProto::InvalidJsonException("Invalid size");

    return sz;
}

QSize getOptionalSize(const QJsonObject& json, const QString& key)
{
    if (json.contains(key))
        return getRequiredSize(json, key);

    return QSize();
}

const QString TenorGif::getUrlForPosting() const
{
    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mUrl);
    QUrlQuery query{
        {"hh", QString::number(mSize.height())},
        {"ww", QString::number(mSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

QJsonObject TenorGif::toJson() const
{
    QJsonObject json;
    json.insert("id", mId);
    json.insert("description", mDescription);
    json.insert("searchTerm", mSearchTerm);
    json.insert("url", mUrl);
    jsonInsert(json, "size", mSize);
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
            getOptionalSize(json, "size"),
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

void TenorGif::fixSize()
{
    if (!mSize.isEmpty())
        return;

    Q_ASSERT(!mSmallSize.isEmpty());
    const qreal ratio = 498.0 / mSmallSize.width(); // 498 is a common width for gifs.
    mSize = mSmallSize * ratio;

    qDebug() << "No valid size set, estimated size:" << mSize;
}

}
