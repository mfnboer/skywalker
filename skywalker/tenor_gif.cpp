// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

const QString TenorGif::getUrlForPosting(QSize fallbackSize) const
{
    const QSize gifSize = mPrivate->mSize.isEmpty() ? fallbackSize : mPrivate->mSize;
    qDebug() << "Gif size:" << gifSize;

    if (gifSize.isEmpty())
        return mPrivate->mUrl;

    // Without these parameters Bluesky will not play the GIF.
    QUrl gifUrl(mPrivate->mUrl);
    QUrlQuery query{
        {"hh", QString::number(gifSize.height())},
        {"ww", QString::number(gifSize.width())}
    };
    gifUrl.setQuery(query);
    return gifUrl.toString();
}

TenorGif TenorGif::deepCopy() const
{
    TenorGif copy;
    *copy.mPrivate = *mPrivate;
    return copy;
}

}
