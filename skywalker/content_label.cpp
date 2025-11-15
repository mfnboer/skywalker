// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_label.h"

namespace Skywalker {

const std::unordered_set<QString>& ContentLabel::getOverridableSystemLabelIds()
{
    // These labels can be set by any labeler. As not all 3rd party labelers can
    // be trusted, a user may want to override their visibility.
    static const std::unordered_set<QString> LABELS{ "!hide", "!warn" };
    return LABELS;
}

bool ContentLabel::isOverridableSytemLabelId(const QString& labelId)
{
    return getOverridableSystemLabelIds().contains(labelId);
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
