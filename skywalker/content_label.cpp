// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_label.h"

namespace Skywalker {

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
