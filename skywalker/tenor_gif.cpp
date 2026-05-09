// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "tenor_gif.h"

namespace Skywalker {

static QString getMediaBaseName(const QString& mediaUrl)
{
    if (mediaUrl.isEmpty())
        return {};

    const QStringList parts = mediaUrl.split('/');

    if (parts.empty())
        return {};

    const QString& lastPart = parts.back();

    const QStringList nameParts = lastPart.split('.');

    if (nameParts.size() != 2)
        return {};

    return nameParts.front();
}

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

    // These can be used to play mp4 or webm instead of GIF.
    // Bluesky adds these too.

    const QString mp4BaseName = getMediaBaseName(mPrivate->mMp4Url);

    if (!mp4BaseName.isEmpty())
        query.addQueryItem("mp4", mp4BaseName);

    const QString webmBaseName = getMediaBaseName(mPrivate->mWebmUrl);

    if (!webmBaseName.isEmpty())
        query.addQueryItem("webm", webmBaseName);

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
