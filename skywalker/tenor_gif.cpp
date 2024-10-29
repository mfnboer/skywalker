// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

const QString TenorGif::getUrlForPosting() const
{
    if (mPrivateData->mSize.isEmpty())
        return mPrivateData->mUrl;

    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mPrivateData->mUrl);
    QUrlQuery query{
        {"hh", QString::number(mPrivateData->mSize.height())},
        {"ww", QString::number(mPrivateData->mSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

}
