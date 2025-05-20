// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "uri_with_expiry.h"
#include <atproto/lib/xjson.h>

namespace Skywalker {

UriWithExpiry::UriWithExpiry(const QString& uri, const QDateTime& expiry) :
    mUri(uri),
    mExpiry(expiry)
{
}

bool UriWithExpiry::operator<(const UriWithExpiry &other) const
{
    if  (mExpiry != other.mExpiry)
        return mExpiry < other.mExpiry;

    return mUri < other.mUri;
}

QJsonObject UriWithExpiry::toJson() const
{
    QJsonObject json;
    json.insert("uri", mUri);
    json.insert("expiry", mExpiry.toString(Qt::ISODateWithMs));
    return json;
}

UriWithExpiry UriWithExpiry::fromJson(const QJsonObject& json)
{
    const ATProto::XJsonObject xjson(json);
    UriWithExpiry uriWithExpiry;
    uriWithExpiry.mUri = xjson.getRequiredString("uri");
    uriWithExpiry.mExpiry = xjson.getRequiredDateTime("expiry");
    return uriWithExpiry;
}

UriWithExpirySet::UriWithExpirySet(QObject* parent) : QObject(parent)
{
}

void UriWithExpirySet::clear()
{
    mUriMap.clear();
    mUrisByExpiry.clear();
}

void UriWithExpirySet::insert(const UriWithExpiry& uriWithExpiry)
{
    remove(uriWithExpiry.getUri());
    auto result = mUrisByExpiry.insert(uriWithExpiry);
    mUriMap[uriWithExpiry.getUri()] = result.first;
}

void UriWithExpirySet::remove(const QString& uri)
{
    if (mUriMap.contains(uri))
    {
        auto it = mUriMap[uri];
        mUriMap.erase(uri);
        mUrisByExpiry.erase(it);
    }
}

QDateTime UriWithExpirySet::getExpiry(const QString& uri) const
{
    auto it = mUriMap.find(uri);
    return it != mUriMap.end() ? it->second->getExpiry() : QDateTime{};
}

const UriWithExpiry* UriWithExpirySet::getFirstExpiry() const
{
    if (mUrisByExpiry.empty())
        return nullptr;

    return &*mUrisByExpiry.begin();
}

QJsonArray UriWithExpirySet::toJson() const
{
    QJsonArray jsonArray;

    for (const auto& uriWithExpiry : mUrisByExpiry)
    {
        const QJsonObject json = uriWithExpiry.toJson();
        jsonArray.push_back(json);
    }

    return jsonArray;
}

void UriWithExpirySet::fromJson(const QJsonArray& jsonArray)
{
    clear();

    for (const auto& json : jsonArray)
    {
        try {
            const auto uriWithExpiry = UriWithExpiry::fromJson(json.toObject());
            insert(uriWithExpiry);
        } catch (ATProto::InvalidJsonException& e) {
            qWarning() << "Invalid UriWithExpiry::Set:" << e.msg();
        }
    }
}

}
