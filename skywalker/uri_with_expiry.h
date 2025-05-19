// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QDateTime>
#include <QObject>
#include <set>

namespace Skywalker {

class UriWithExpiry
{
public:
    UriWithExpiry() = default;
    ~UriWithExpiry() = default;
    UriWithExpiry(const UriWithExpiry&) = default;
    UriWithExpiry& operator=(const UriWithExpiry&) = default;

    UriWithExpiry(const QString& uri, const QDateTime& expiry);

    const QString& getUri() const { return mUri; }
    const QDateTime& getExpiry() const { return mExpiry; }

    void setUri(const QString& uri) { mUri = uri; }
    void setExpiry(const QDateTime& expiry) { mExpiry = expiry; }

    bool operator<(const UriWithExpiry &other) const;
    bool operator==(const UriWithExpiry &other) const = default;

    QJsonObject toJson() const;
    static UriWithExpiry fromJson(const QJsonObject& json);

private:
    QString mUri;
    QDateTime mExpiry;
};

class UriWithExpirySet : public QObject
{
    Q_OBJECT

public:
    explicit UriWithExpirySet(QObject* parent = nullptr);

    void clear();
    void insert(const UriWithExpiry& uriWithExpiry);
    void remove(const QString& uri);
    Q_INVOKABLE QDateTime getExpiry(const QString& uri) const;
    const UriWithExpiry* getFirstExpiry() const;

    QJsonArray toJson() const;
    void fromJson(const QJsonArray& jsonArray);


private:
    std::set<UriWithExpiry> mUrisByExpiry;
    std::unordered_map<QString, std::set<UriWithExpiry>::iterator> mUriMap;
};

}
