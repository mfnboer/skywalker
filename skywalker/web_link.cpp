// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "web_link.h"

namespace Skywalker {

WebLink::WebLink(const QString& link, int startIndex, int endIndex, const QString& name) :
    mLink(link),
    mName(name),
    mStartIndex(startIndex),
    mEndIndex(endIndex)
{
}

}
