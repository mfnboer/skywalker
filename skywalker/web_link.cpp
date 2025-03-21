// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "web_link.h"
#include <QDebug>

namespace Skywalker {

WebLink::WebLink(const QString& link, int startIndex, int endIndex, const QString& name) :
    mLink(link),
    mName(name),
    mStartIndex(startIndex),
    mEndIndex(endIndex)
{
}

void WebLink::addToIndexes(int add)
{
    mStartIndex += add;
    mEndIndex += add;
}

bool WebLink::isValidEmbeddedLink() const
{
    if (getName().isEmpty())
    {
        qWarning() << "Embedded link must have name:" << getLink();
        return false;
    }

    if (getStartIndex() < 0 || getEndIndex() < 0)
    {
        qWarning() << "Embedded link must have indices:" << getLink() << "start:" << getStartIndex() << "end:" << getEndIndex();
        return false;
    }

    if (getStartIndex() > getEndIndex())
    {
        qWarning() << "Invalid indices:" << getLink() << "start:" << getStartIndex() << "end:" << getEndIndex();
        return false;
    }

    return true;
}

}
