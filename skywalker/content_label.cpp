// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_label.h"
#include "content_filter.h"

namespace Skywalker {

bool ContentLabel::isSystemLabel() const
{
    return ContentFilter::isSystemLabelId(mPrivate->mLabelId);
}

bool ContentLabel::isOverridableSytemLabel() const
{
    return ContentFilter::isOverridableSytemLabelId(mPrivate->mLabelId);
}

bool ContentLabel::appliesToActor() const
{
    return mPrivate->mUri.startsWith("did:");
}

const QString& ContentLabel::getActorDid() const
{
    if (mPrivate->mUri.startsWith("did:"))
        return mPrivate->mUri;

    static const QString NULL_STRING;
    return NULL_STRING;
}

}
