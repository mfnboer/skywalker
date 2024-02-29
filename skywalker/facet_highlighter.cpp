// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "facet_highlighter.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

FacetHighlighter::FacetHighlighter(QTextDocument* parent) :
    EmojiFixHighlighter(parent)
{
}

void FacetHighlighter::setHighlightColor(const QString& colorName)
{
    const auto color = QColor::fromString(colorName);
    mHighlightFormat.setForeground(color);
}

void FacetHighlighter::highlightBlock(const QString& text)
{
    EmojiFixHighlighter::highlightBlock(text);

    // NOTE: unfortunately the text does not contain text from the preedit buffer.
    const auto facets = ATProto::RichTextMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        switch (facet.mType)
        {
        case ATProto::RichTextMaster::ParsedMatch::Type::MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::LINK:
        case ATProto::RichTextMaster::ParsedMatch::Type::TAG:
        {
            const int facetLength = facet.mEndIndex - facet.mStartIndex;
            addFormat(facet.mStartIndex, facetLength, mHighlightFormat);
            break;
        }
        case ATProto::RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }
}

}
