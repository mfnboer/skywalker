// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "uri_with_expiry.h"


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

}
