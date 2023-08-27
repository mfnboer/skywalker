// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QtQmlIntegration>

namespace Skywalker
{

class ImageView
{
    Q_GADGET
    Q_PROPERTY(QString thumbUrl READ getThumbUrl FINAL)
    Q_PROPERTY(QString fullSizeUrl READ getFullSizeUrl FINAL)
    Q_PROPERTY(QString alt READ getAlt FINAL)
    QML_VALUE_TYPE(imageview)

public:
    using Ptr = std::unique_ptr<ImageView>;

    ImageView() = default;
    ImageView(const QString& thumbUrl, const QString& fullSizeUrl, const QString& alt) :
        mThumbUrl(thumbUrl),
        mFullSizeUrl(fullSizeUrl),
        mAlt(alt)
    {}

    const QString& getThumbUrl() const { return mThumbUrl; }
    const QString& getFullSizeUrl() const { return mFullSizeUrl; }
    const QString& getAlt() const { return mAlt; }

private:
    QString mThumbUrl;
    QString mFullSizeUrl;
    QString mAlt;
};

}

Q_DECLARE_METATYPE(Skywalker::ImageView)
