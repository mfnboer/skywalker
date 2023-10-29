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
    // NOTE: unfortunately the text does not contain text from the preedit buffer.
    const auto facets = ATProto::PostMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        switch (facet.mType)
        {
        case ATProto::PostMaster::ParsedMatch::Type::MENTION:
        case ATProto::PostMaster::ParsedMatch::Type::LINK:
        {
            const int facetLength = facet.mEndIndex - facet.mStartIndex;
            setFormat(facet.mStartIndex, facetLength, mHighlightColor);
            break;
        }
        case ATProto::PostMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::PostMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }
}

}
