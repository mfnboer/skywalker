// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "emoji_fix_highlighter.h"
#include "font_downloader.h"
#include "unicode_fonts.h"

namespace Skywalker {

EmojiFixHighlighter::EmojiFixHighlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
    mEmojiFormat.setFont(FontDownloader::getEmojiFont());
}

void EmojiFixHighlighter::setMaxLength(int maxLength, const QString& lengthExceededColor)
{
    mMaxLength = maxLength;
    mLengthExceededFormat.setBackground(QColor(lengthExceededColor));
}

void EmojiFixHighlighter::addFormat(int start, int sz, const QTextCharFormat& fmt)
{
    const int end = start + sz;

    for (int i = start; i < end; ++i)
    {
        auto f = format(i);
        f.merge(fmt);
        setFormat(i, 1, f);
    }
}

void EmojiFixHighlighter::highlightBlock(const QString& text)
{
    highlightLengthExceeded(text);
    setEmojiFontKeycaps(text);
    setEmojiFontCombinedEmojis(text);
}

void EmojiFixHighlighter::highlightLengthExceeded(const QString& text)
{
    if (mMaxLength == -1)
        return;

    int totalLength = 0;
    const int prevLength = previousBlockState();

    if (currentBlock().blockNumber() > 0)
    {
        if (prevLength == -1)
            return;

        totalLength = prevLength + 1; // +1 for newline
    }

    const auto graphemeInfo = UnicodeFonts::getGraphemeInfo(text);
    const int blockLength = graphemeInfo.getLength();
    totalLength += blockLength;
    qDebug() << "BLOCK:" << currentBlock().blockNumber() << "LEN:" << blockLength << currentBlock().length() << "TOTAL:" << totalLength;
    setCurrentBlockState(totalLength);

    if (totalLength <= mMaxLength)
    {
        QTextCharFormat fmt;
        fmt.setFont(document()->defaultFont());
        setFormat(0, text.length(), fmt);
    }
    else if (prevLength >= mMaxLength)
    {
        setFormat(0, text.length(), mLengthExceededFormat);
    }
    else
    {
        const int inMaxGraphemes = mMaxLength - prevLength;
        Q_ASSERT(inMaxGraphemes > 0);
        const int charPos = graphemeInfo.getCharPos(inMaxGraphemes - 1);
        QTextCharFormat fmt;
        fmt.setFont(document()->defaultFont());
        setFormat(0, charPos, fmt);

        const auto exceededLen = text.length() - charPos;
        setFormat(charPos, exceededLen, mLengthExceededFormat);
    }
}

void EmojiFixHighlighter::setEmojiFontKeycaps(const QString& text)
{
    static const QRegularExpression enclosingKeycapRE("(.\uFE0F\u20E3)");
    auto i = text.indexOf(enclosingKeycapRE);

    while (i != -1)
    {
        addFormat(i, 3, mEmojiFormat);
        i = text.indexOf(enclosingKeycapRE, i + 3);
    }
}

void EmojiFixHighlighter::setEmojiFontCombinedEmojis(const QString& text)
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
                addFormat(prev, len, mEmojiFormat);
        }

        prev = next;
    }
}

}
