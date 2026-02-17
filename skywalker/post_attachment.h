// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "link_card.h"
#include <QList>
#include <QString>

namespace Skywalker {

struct PostAttachmentImages
{
    QStringList mFileNames;
    QStringList mAltTexts;
};

struct PostAttachmentLinkCard
{
    const LinkCard* mLinkCard;
};

struct PostAttachmentVideo
{
    QString mResource; // file://name or http-link
    QString mAltText;
    int mWidth;
    int mHeight;
    bool mIsGif = false;
};

using PostAttachment = std::variant<PostAttachmentImages, PostAttachmentLinkCard, PostAttachmentVideo>;

}
