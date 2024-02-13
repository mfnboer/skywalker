// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "facet_highlighter.h"
#include "font_downloader.h"
#include "unicode_fonts.h"
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

    setEmojiFontKeycaps(text);
    setEmojiFontCombinedEmojis(text);
}

void FacetHighlighter::setEmojiFontKeycaps(const QString& text)
{
    static const QRegularExpression enclosingKeycapRE("(.\uFE0F\u20E3)");
    auto i = text.indexOf(enclosingKeycapRE);

    while (i != -1)
    {
        setFormat(i, 3, FontDownloader::getEmojiFont());
        i = text.indexOf(enclosingKeycapRE, i + 3);
    }
}

void FacetHighlighter::setEmojiFontCombinedEmojis(const QString& text)
{
    // ZWJ Emoji's are not always correctly rendered. Somehow the primary font
    // renders them as 2 separate emoji's.
    //
    // Example: the rainbow flag: \U0001F3F3\uFE0F\u200D\U0001F308"
    //          \U0001F3F3\uFE0F = white flag
    //          \u200D =           ZWJ
    //          \U0001F308 =       rainbow
    //
    // Explicity set emoji font for long emoji graphemes.

    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    int prev = 0;
    int next;

    while ((next = boundaryFinder.toNextBoundary()) != -1)
    {
        const int len = next - prev;

        if (len > 2)
        {
            const QString grapheme = text.sliced(prev, len);

            if (UnicodeFonts::onlyEmojis(grapheme))
                setFormat(prev, len, FontDownloader::getEmojiFont());
        }

        prev = next;
    }
}

}
