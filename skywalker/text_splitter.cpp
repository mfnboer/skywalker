// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "text_splitter.h"
#include "split_text_boundary_finder.h"
#include "unicode_fonts.h"

namespace Skywalker {

TextSplitterPart::TextSplitterPart(const QString& text, const WebLink::List& embeddedLinks) :
    mText(text),
    mEmbeddedLinks(embeddedLinks)
{
}

TextSplitter::TextSplitter(QObject* parent) : QObject(parent)
{
}

static void moveShortLineToNextPart(QString& part, int maxLength, int minSplitLineLength)
{
    if (part.back() == '\n')
        return;

    const int lastNewLine = part.lastIndexOf('\n');

    if (part.size() > maxLength - minSplitLineLength && lastNewLine >= part.size() - minSplitLineLength)
        part = part.sliced(0, lastNewLine + 1);
}

static WebLink::List getLinksForPart(const WebLink::List& embeddedLinks, int partStartPos, int partSize)
{
    WebLink::List partLinks;
    const int partEndPos = partStartPos + partSize;

    for (const auto& link : embeddedLinks)
    {
        if (link.getStartIndex() >= partStartPos && link.getEndIndex() <= partEndPos)
        {
            partLinks.push_back(link);

            // Adjust link indexes to the start of the part
            auto& addedLink = partLinks.back();
            addedLink.addToIndexes(-partStartPos);
        }
    }

    return partLinks;
}

TextSplitterPart::List TextSplitter::splitText(
        const QString& text, const WebLink::List& embeddedLinks,
        int maxLength, int minSplitLineLength, int maxParts) const
{
    if (text.size() <= maxLength)
        return {TextSplitterPart(text, embeddedLinks)};

    TextSplitterPart::List parts;
    SplitTextBoundaryFinder boundaryFinder(text, embeddedLinks);
    int startPartPos = 0;
    int startLinePos = 0;
    int endLinePos = 0;

    while (!(boundaryFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem) && startLinePos != -1)
        startLinePos = boundaryFinder.toNextBoundary();

    qDebug() << "start:" << startLinePos;

    while (startLinePos != -1 && parts.size() < maxParts - 1)
    {
        const int nextEndLinePos = boundaryFinder.toNextBoundary();
        qDebug() << "next end:" << nextEndLinePos;

        if (nextEndLinePos == -1)
            break;

        const int partLength = nextEndLinePos - startPartPos;
        QString part = text.sliced(startPartPos, partLength);

        if (part.size() == maxLength)
        {
            moveShortLineToNextPart(part, maxLength, minSplitLineLength);
            qDebug() << QString("Part: [%1]").arg(part);
            const auto partLinks = getLinksForPart(embeddedLinks, startPartPos, part.size());
            parts.push_back(TextSplitterPart(part, partLinks));
            startPartPos += part.size();
        }
        else if (part.size() > maxLength)
        {
            QString prevPart = text.sliced(startPartPos, endLinePos - startPartPos);

            if (!prevPart.isEmpty())
            {
                moveShortLineToNextPart(prevPart, maxLength, minSplitLineLength);
                qDebug() << QString("Prev part: [%1]").arg(prevPart);
                const auto partLinks = getLinksForPart(embeddedLinks, startPartPos, prevPart.size());
                parts.push_back(TextSplitterPart(prevPart, partLinks));
                startPartPos += prevPart.size();
            }
            else
            {
                qDebug() << "Empty part";
                startPartPos = endLinePos;
            }
        }

        endLinePos = nextEndLinePos;
        startLinePos = endLinePos;
    }

    if (startPartPos < text.size())
    {
        const QString part = text.sliced(startPartPos);
        qDebug() << QString("Last part: [%1]").arg(part);
        const auto partLinks = getLinksForPart(embeddedLinks, startPartPos, part.size());
        parts.push_back(TextSplitterPart(part, partLinks));
    }

    return parts;
}

TextSplitterPart TextSplitter::joinText(
    const QString& text1, const WebLink::List& embeddedLinks1,
    const QString& text2, const WebLink::List& embeddedLinks2) const
{
    if (text1.isEmpty())
        return TextSplitterPart(text2, embeddedLinks2);

    if (text2.isEmpty())
        return TextSplitterPart(text1, embeddedLinks1);

    QString joinedText = text1;
    WebLink::List joinedLinks = embeddedLinks1;

    if (UnicodeFonts::hasPhraseEnding(text1))
        joinedText += "\n\n";
    else if (!text1.back().isSpace() && !text2.front().isSpace())
        joinedText += ' ';

    const int indexShift = joinedText.size();
    joinedText += text2;

    for (const auto& link : embeddedLinks2)
    {
        joinedLinks.push_back(link);
        auto& addedLink = joinedLinks.back();
        addedLink.addToIndexes(indexShift);
    }

    return TextSplitterPart(std::move(joinedText), std::move(joinedLinks));
}

}
