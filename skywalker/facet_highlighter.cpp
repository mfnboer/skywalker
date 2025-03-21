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

void FacetHighlighter::setEmbeddedLinks(const WebLink::List* links)
{
    mEmbeddedLinks = links;
}

void FacetHighlighter::highlightBlock(const QString& text)
{
    EmojiFixHighlighter::highlightBlock(text);

    // NOTE: unfortunately the text does not contain text from the preedit buffer.
    const auto facets = ATProto::RichTextMaster::parseFacets(text);

    for (const auto& facet : facets)
    {
        if (facetOverlapsWithEmbeddedLink(facet))
            continue;

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

    highlightEmbeddedLinks(text);
}

bool FacetHighlighter::facetOverlapsWithEmbeddedLink(const ATProto::RichTextMaster::ParsedMatch& facet) const
{
    if (!mEmbeddedLinks)
        return false;

    const int prevLength = getPrevBlockTotalCharLength();

    if (prevLength < 0)
        return false;

    for (const auto& link : *mEmbeddedLinks)
    {
        const int startIndex = link.getStartIndex() - prevLength;

        if (startIndex < 0)
        {
            qWarning() <<"Invalid index:" << link.getName() << "start:" << link.getStartIndex() << "prev:" << prevLength;
            continue;
        }

        const int endIndex = link.getEndIndex() - prevLength;

        if (facet.mStartIndex < endIndex && facet.mEndIndex > startIndex)
            return true;
    }

    return false;
}

void FacetHighlighter::highlightEmbeddedLinks(const QString& text)
{
    if (!mEmbeddedLinks)
        return;

    const int prevLength = getPrevBlockTotalCharLength();

    if (prevLength < 0)
        return;

    for (const auto& link : *mEmbeddedLinks)
    {
        const int startIndex = link.getStartIndex() - prevLength;

        if (startIndex < 0 || startIndex >= text.size())
        {
            qDebug() << "Link in other block:" << link.getName() << "start:" << link.getStartIndex() << "prev:" << prevLength << "block:" << currentBlock().blockNumber();
            continue;
        }

        const int facetLength = link.getEndIndex() - link.getStartIndex();
        addFormat(startIndex, facetLength, mHighlightFormat);
    }
}

}
