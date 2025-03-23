// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "split_text_boundary_finder.h"

namespace Skywalker {

SplitTextBoundaryFinder::SplitTextBoundaryFinder(const QString& text, const WebLink::List& embeddedLinks) :
    mBoundaryFinder(QTextBoundaryFinder::Line, text),
    mEmbeddedLinks(embeddedLinks)
{
}

bool SplitTextBoundaryFinder::isInsideEmbeddedLinks(int index) const
{
    for (const auto& link : mEmbeddedLinks)
    {
        if (index > link.getStartIndex() && index < link.getEndIndex())
            return true;
    }

    return false;
}

int SplitTextBoundaryFinder::toNextBoundary()
{
    int next = mBoundaryFinder.toNextBoundary();

    while (next >= 0 && isInsideEmbeddedLinks(next))
        next = mBoundaryFinder.toNextBoundary();

    return next;
}

}
