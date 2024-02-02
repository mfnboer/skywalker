// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "facet_highlighter.h"
#include "font_downloader.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

FacetHighlighter::FacetHighlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
}

void FacetHighlighter::setHighlightColor(const QString& colorName)
{
    mHighlightColor = QColor::fromString(colorName);
}

void FacetHighlighter::highlightBlock(const QString& text)
{
    // NOTE: unfortunately the text does not contain text from the preedit buffer.
    const auto facets = ATProto::RichTextMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        switch (facet.mType)
        {
        case ATProto::RichTextMaster::ParsedMatch::Type::MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::LINK:
        {
            const int facetLength = facet.mEndIndex - facet.mStartIndex;
            setFormat(facet.mStartIndex, facetLength, mHighlightColor);
            break;
        }
        case ATProto::RichTextMaster::ParsedMatch::Type::TAG:
        case ATProto::RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION:
        case ATProto::RichTextMaster::ParsedMatch::Type::UNKNOWN:
            break;
        }
    }

    static const QRegularExpression enclosingKeycapRE("(.\uFE0F\u20E3)");
    auto i = text.indexOf(enclosingKeycapRE);

    while (i != -1)
    {
        setFormat(i, 3, FontDownloader::getEmojiFont());
        i = text.indexOf(enclosingKeycapRE, i + 3);
    }
}

}
