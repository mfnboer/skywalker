// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker
{
class ImageView
{
    Q_GADGET
    Q_PROPERTY(QString thumbUrl MEMBER mThumbUrl CONSTANT)
    Q_PROPERTY(QString fullSizeUrl MEMBER mFullSizeUrl CONSTANT)
    Q_PROPERTY(QString alt MEMBER mAlt CONSTANT)

public:
    using Ptr = std::unique_ptr<ImageView>;

    ImageView() = default;
    ImageView(const ImageView&) = default;
    ImageView(const QString& thumbUrl, const QString& fullSizeUrl, const QString& alt) :
        mThumbUrl(thumbUrl),
        mFullSizeUrl(fullSizeUrl),
        mAlt(alt)
    {}

    const QString mThumbUrl;
    const QString mFullSizeUrl;
    const QString mAlt;
};

}

Q_DECLARE_METATYPE(Skywalker::ImageView)
