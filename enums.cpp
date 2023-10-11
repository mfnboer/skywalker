// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "enums.h"

namespace Skywalker {

QEnums::ContentPrefVisibility QEnums::toContentPrefVisibility(ContentVisibility visibilty)
{
    switch (visibilty)
    {
    case CONTENT_VISIBILITY_SHOW:
        return CONTENT_PREF_VISIBILITY_SHOW;
    case CONTENT_VISIBILITY_WARN_MEDIA:
    case CONTENT_VISIBILITY_WARN_POST:
        return CONTENT_PREF_VISIBILITY_WARN;
    case CONTENT_VISIBILITY_HIDE_MEDIA:
    case CONTENT_VISIBILITY_HIDE_POST:
        return CONTENT_PREF_VISIBILITY_HIDE;
    }

    Q_ASSERT(false);
    return CONTENT_PREF_VISIBILITY_HIDE;
}

}
