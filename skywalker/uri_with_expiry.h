// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QDateTime>
#include <QString>
#include <set>

namespace Skywalker {

class UriWithExpiry
{
public:
    using Set = std::set<UriWithExpiry>;

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

private:
    QString mUri;
    QDateTime mExpiry;
};

}

Q_DECLARE_METATYPE(::Skywalker::UriWithExpiry)
Q_DECLARE_METATYPE(::Skywalker::UriWithExpiry::Set)
