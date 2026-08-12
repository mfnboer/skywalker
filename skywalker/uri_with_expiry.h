// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
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

    UriWithExpiry(const QString& uri, const QDateTime& expiry, bool onlyReposts, bool onlyQuotes);

    const QString& getUri() const { return mUri; }
    const QDateTime& getExpiry() const { return mExpiry; }
    bool isOnlyReposts() const { return mOnlyReposts; }
    bool isOnlyQuotePosts() const { return mOnlyQuotes; }

    bool operator<(const UriWithExpiry &other) const;
    bool operator==(const UriWithExpiry &other) const;

    QJsonObject toJson() const;
    static UriWithExpiry fromJson(const QJsonObject& json);

private:
    QString mUri;
    QDateTime mExpiry;

    // Scoped mute to set after expiry
    bool mOnlyReposts = false;
    bool mOnlyQuotes = false;
};

class UriWithExpirySet : public QObject
{
    Q_OBJECT

public:
    explicit UriWithExpirySet(QObject* parent = nullptr);

    void clear();
    void insert(const UriWithExpiry& uriWithExpiry);
    bool remove(const QString& uri);
    const UriWithExpiry* get(const QString& uri) const;
    Q_INVOKABLE QDateTime getExpiry(const QString& uri) const;
    const UriWithExpiry* getFirstExpiry() const;

    QJsonArray toJson() const;
    void fromJson(const QJsonArray& jsonArray);


private:
    std::set<UriWithExpiry> mUrisByExpiry;
    std::unordered_map<QString, std::set<UriWithExpiry>::iterator> mUriMap;
};

}
