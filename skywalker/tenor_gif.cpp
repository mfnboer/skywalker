// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

const QString TenorGif::getUrlForPosting() const
{
    if (mData->mSize.isEmpty())
        return mData->mUrl;

    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mData->mUrl);
    QUrlQuery query{
        {"hh", QString::number(mData->mSize.height())},
        {"ww", QString::number(mData->mSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

}
