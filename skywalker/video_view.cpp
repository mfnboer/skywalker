// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "video_view.h"
#include "cached_image_provider.h"

namespace Skywalker {

QString VideoView::getThumbUrl() const
{
    static const auto* provider = CachedImageProvider::getProvider(CachedImageProvider::THUMB_IMG);
    const QString thumbUrl = QUrl::fromPercentEncoding(getThumbHttpUrl().toUtf8());
    return provider->createImageUrl(thumbUrl);
}


}