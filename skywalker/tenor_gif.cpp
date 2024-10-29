// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

const QString TenorGif::getUrlForPosting() const
{
    if (mPrivate->mSize.isEmpty())
        return mPrivate->mUrl;

    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mPrivate->mUrl);
    QUrlQuery query{
        {"hh", QString::number(mPrivate->mSize.height())},
        {"ww", QString::number(mPrivate->mSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

}
