// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "content_label.h"

namespace Skywalker {

bool ContentLabel::appliesToActor() const
{
    return mUri.startsWith("did:");
}

QString ContentLabel::getActorDid() const
{
    if (mUri.startsWith("did:"))
        return mUri;

    return {};
}

}
