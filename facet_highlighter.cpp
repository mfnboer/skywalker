// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "facet_highlighter.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

FacetHighlighter::FacetHighlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
}

void FacetHighlighter::setHighlightColor(const QString& colorName)
{
    mHighlightColor.setNamedColor(colorName);
}

void FacetHighlighter::highlightBlock(const QString& text)
{
    const auto facets = ATProto::PostMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        const int facetLength = facet.mEndIndex - facet.mStartIndex;
        setFormat(facet.mStartIndex, facetLength, mHighlightColor);
    }
}

}
