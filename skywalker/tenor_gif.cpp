// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

const QString TenorGif::getUrlForPosting() const
{
    if (mSize.isEmpty())
        return mUrl;

    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mUrl);
    QUrlQuery query{
        {"hh", QString::number(mSize.height())},
        {"ww", QString::number(mSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

}
