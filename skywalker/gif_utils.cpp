// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "gif_utils.h"
#include "cached_image_provider.h"
#include <QUrl>

namespace Skywalker {

namespace {
constexpr char const* TENOR_PREFIX = "https://tenor.com/";
constexpr char const* TENOR_MEDIA_PREFIX = "https://media.tenor.com/";

constexpr char const* GRAYSKY_PREFIX = "https://graysky.app/gif/";

constexpr char const* GIPHY_GIFS_PREFIX = "https://giphy.com/gifs/";
constexpr char const* GIPHY_DOMAIN = "giphy.com";

constexpr char const* KLIPY_PREFIX = "https://static.klipy.com/";
}

GifUtils::GifUtils(QObject* parent) :
    QObject(parent)
{
}

bool GifUtils::isTenorLink(const QString& link) const
{
    return link.startsWith(TENOR_PREFIX) ||
           link.startsWith(TENOR_MEDIA_PREFIX) ||
           link.startsWith(GRAYSKY_PREFIX);
}

bool GifUtils::isGiphyLink(const QString& link) const
{
    return link.contains(GIPHY_DOMAIN);
}

bool GifUtils::isKlipyLink(const QString& link) const
{
    return link.startsWith(KLIPY_PREFIX);
}

Q_INVOKABLE bool GifUtils::isGif(const QString& link) const
{
    return !getGifUrl(link).isEmpty();
}

QString GifUtils::getGifUrl(const QString& link) const
{
    if (link.startsWith(KLIPY_PREFIX))
        return getKlipyGifUrl(link);

    if (link.startsWith(GIPHY_GIFS_PREFIX))
        return getGiphyGifUrl(link);

    // E.g. https://media.giphy.com/media/, https://media0.giphy.com/media/
    if (link.startsWith("https://media") && link.contains(GIPHY_DOMAIN + QString("/media/")))
        return getGiphyMediaUrl(link);

    if (link.startsWith(TENOR_PREFIX))
        return getTenorViewGif(link);

    if (link.startsWith(GRAYSKY_PREFIX))
    {
        const QString tenorLink = convertGrayskyToTenor(link);
        return tenorLink.isEmpty() ? "" : getTenorMediaGif(tenorLink);
    }

    if (link.startsWith(TENOR_MEDIA_PREFIX))
        return getTenorMediaGif(link);

    return {};
}

QString GifUtils::getGifVideoUrl(const QString& link) const
{
    qDebug() << "Get GIF video URL for:" << link;

    if (!isKlipyLink(link))
        return {};

    const QString gifUrl = getGifUrl(link);

    if (gifUrl.isEmpty())
        return {};

    static const QRegularExpression RE_KLIPY{R"(^(.+)\/([[:alnum:]_]+)\.gif$)"};
    auto match = RE_KLIPY.matchView(gifUrl);

    if (!match.hasMatch())
    {
        qWarning() << "Cannot match Klipy URL:" << gifUrl;
        return {};
    }

    const QUrl linkUrl(link);
    const QUrlQuery query(linkUrl.query());

    for (const auto& format : {"webm", "mp4"})
    {
        const auto& formatId = query.queryItemValue(format);

        if (formatId.isEmpty())
            continue;

        const auto prefix = match.captured(1);
        const auto videoUrl = QString("%1/%2.%3").arg(prefix, formatId, format);
        qDebug() << "Video:" << format << "url:" << videoUrl;
        return videoUrl;
    }

    return {};
}

VideoView GifUtils::getGifVideoView(const QString& link, const QString& title, const QString& description, const QString& thumbUrl) const
{
    const QString& videoUrl = getGifVideoUrl(link);

    if (videoUrl.isEmpty())
        return {};

    auto video = std::make_shared<ATProto::AppBskyEmbed::VideoView>();
    video->mPlaylist = videoUrl;
    video->mPresentation = ATProto::AppBskyEmbed::VideoPresentation::GIF;

    // HACK: Alt text for a GIF is stored in the description with this prefix "Alt: ".
    if (description.startsWith("Alt: "))
        video->mAlt = description.sliced(5);
    else if (!description.isEmpty())
        video->mAlt = description;
    else if (!title.isEmpty())
        video->mAlt = title;

    if (!thumbUrl.isEmpty())
    {
        const auto thumb = thumbUrl.startsWith("image://") ? CachedImageProvider::createHttpUrl(thumbUrl) : thumbUrl;
        video->mThumbnail = thumb;
    }

    const QSize gifSize = getGifSize(link);

    if (gifSize.isValid())
    {
        video->mAspectRatio = std::make_shared<ATProto::AppBskyEmbed::AspectRatio>();
        video->mAspectRatio->mWidth = gifSize.width();
        video->mAspectRatio->mHeight = gifSize.height();
    }

    return VideoView(video);
}

QSize GifUtils::getGifSize(const QString& link) const
{
    qDebug() << link;
    const QUrl url(link);

    if (!url.hasQuery())
        return QSize{-1, -1};

    const QUrlQuery query(url.query());

    // URL in post has ww and hh as size tags.
    const QSize sz = getGifSize(query, "ww", "hh");

    if (sz.isValid())
        return sz;

    // URL in draft has gifwidth and gifHeight as tags
    return getGifSize(query, "gifWidth", "gifHeight");
}

QSize GifUtils::getGifSize(const QUrlQuery& query, const QString& widthTag, const QString& heightTag) const
{
    // These query parameters are available in Tenor links
    if (!query.hasQueryItem(heightTag) || !query.hasQueryItem(widthTag))
        return QSize{-1, -1};

    const int width = query.queryItemValue(widthTag).toInt();

    if (width <= 0)
        return QSize{-1, -1};

    const int height = query.queryItemValue(heightTag).toInt();

    if (height <= 0)
        return QSize{-1, -1};

    const auto sz = QSize{width, height};
    qDebug() << "Size:" << sz;
    return sz;
}

QString GifUtils::getKlipyGifUrl(const QString& link) const
{
    const QUrl url(link);

    if (!url.isValid())
        return {};

    const QString gifLink = url.toString(QUrl::RemoveQuery);
    return gifLink.endsWith(".gif") ? gifLink : "";
}

// Example: https://giphy.com/gifs/ufc-sport-297-ufc297-ycTrWycYMLlUNoHl73
// Result:  https://i.giphy.com/ycTrWycYMLlUNoHl73.gif
QString GifUtils::getGiphyGifUrl(const QString& link) const
{
    const QUrl url(link);

    if (!url.isValid())
        return {};

    const auto pathParts = url.path().split("/", Qt::SkipEmptyParts);
    if (pathParts.isEmpty())
        return {};

    const auto& gifLongId = pathParts.back();
    const auto gifLongIdParts = gifLongId.split("-");
    const auto& gifId = gifLongIdParts.back();
    const QString gifLink = "https://i.giphy.com/" + gifId + ".gif";

    return gifLink;
}

QString GifUtils::getGiphyMediaUrl(const QString& link) const
{
    const QUrl url(link);

    if (!url.isValid())
        return {};

    const QString gifLink = url.toString(QUrl::RemoveQuery);
    return gifLink.endsWith(".gif") ? gifLink : "";
}

// Example: https://tenor.com/view/happy-thursday-mawnin-gif-8975369713728247911
// Result:  https://tenor.com/view/happy-thursday-mawnin-gif-8975369713728247911.gif
// NOTE: there can be a county code between the host and "view", e.g. https://tenor.com/nl/view/
QString GifUtils::getTenorViewGif(const QString& link) const
{
    const QUrl url(link);

    if (!url.isValid())
        return {};

    const QString path = url.path();
    if (!path.contains("/view/"))
        return {};

    const QString gifLink = url.toString(QUrl::RemoveQuery) + ".gif";
    return gifLink;
}

// Qt6 has a bug that blocks playing video from a https source.
// As a workaround we convert the MP4 url from Tenor to GIF (this may not work forever)
// These are examples of Tenor URL's:
//
// https://media.tenor.com/2w1XsfvQD5kAAAPo/hhgf.mp4 (normal MP4 format)
// https://media.tenor.com/2w1XsfvQD5kAAAAd/hhgf.gif (medium GIF format)
//
// The last 2 characters of the TenorId, e.g. 2w1XsfvQD5kAAAPo indicates the format.
//
// Po = normal MP4
// Ad = medium GIF
QString GifUtils::getTenorMediaGif(const QString& link) const
{
    const QUrl url(link);

    if (!url.isValid())
        return {};

    if (url.path().endsWith(".mp4") || url.path().endsWith(".webm"))
    {
        auto pathParts = url.path().split("/", Qt::SkipEmptyParts);

        if (pathParts.size() < 2)
            return {};

        const QString& tenorId = pathParts[pathParts.size() - 2];

        if (tenorId.size() < 3)
        {
            qWarning() << "Unknown Tenor ID:" << tenorId << "link:" << link;
            return {};
        }

        const QString baseTenorId = tenorId.sliced(0, tenorId.length() - 2);
        const QString fileName = pathParts.back();
        const QString baseFileName = fileName.split(".")[0];

        pathParts[pathParts.size() - 2] = baseTenorId + "Ad";
        pathParts[pathParts.size() - 1] = baseFileName + ".gif";
        const QString gifLink = TENOR_MEDIA_PREFIX + pathParts.join("/");
        return gifLink;
    }

    if (url.path().endsWith(".gif"))
        return link;

    return {};
}

// Graysky wraps Tenor media links
// Example: https://graysky.app/gif/nG9mD0Dl8vsAAAPo/yay.mp4?title=Yay%20GIF
// Tenor:   https://media.tenor.com/nG9mD0Dl8vsAAAPo/yay.mp4
QString GifUtils::convertGrayskyToTenor(const QString& link) const
{
    const QUrl url(link);
    if (!url.isValid())
        return {};

    const QString removedQuery = url.toString(QUrl::RemoveQuery);
    const QString gifUrl = TENOR_MEDIA_PREFIX + removedQuery.sliced(QString(GRAYSKY_PREFIX).length());
    qDebug() << "GIF url:" << gifUrl;
    return gifUrl;
}

}
