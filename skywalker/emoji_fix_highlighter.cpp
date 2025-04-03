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

void EmojiFixHighlighter::setMaxLength(int maxLength)
{
    if (maxLength != mMaxLength)
    {
        qDebug() << "Max length change from" << mMaxLength << "to" << maxLength << ": re-highlight";
        mMaxLength = maxLength;
        rehighlight();
    }
}

void EmojiFixHighlighter::setEmojiFixDocument(QQuickTextDocument* doc, int maxLength, const QString& lengthExceededColor)
{
    setMaxLength(maxLength, lengthExceededColor);
    setDocument(doc->textDocument());
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

    int totalCharLength = 0;
    int totalGraphemeLength = 0;
    const int prevBlockState = previousBlockState();
    const int prevGraphemeLength = prevBlockState >= 0 ? (prevBlockState & 0x0000ffff) : 0;
    const int prevCharLength = prevBlockState >= 0 ? ((prevBlockState >> 16) & 0x0000ffff) : 0;

    if (currentBlock().blockNumber() > 0)
    {
        if (prevBlockState == -1)
            return;

        totalGraphemeLength = prevGraphemeLength + 1; // +1 for newline
        totalCharLength = prevCharLength + 1;
    }

    const auto graphemeInfo = UnicodeFonts::getGraphemeInfo(text);
    const int blockGraphemeLength = graphemeInfo.getLength();
    totalGraphemeLength += blockGraphemeLength;
    totalCharLength += text.length();
    setCurrentBlockState((totalCharLength << 16) | totalGraphemeLength);

    if (totalGraphemeLength <= mMaxLength)
    {
        QTextCharFormat fmt;
        fmt.setFont(document()->defaultFont());
        setFormat(0, text.length(), fmt);
    }
    else if (prevGraphemeLength >= mMaxLength)
    {
        setFormat(0, text.length(), mLengthExceededFormat);
    }
    else
    {
        const int inMaxGraphemes = mMaxLength - prevGraphemeLength;
        Q_ASSERT(inMaxGraphemes > 0);
        const int charPos = graphemeInfo.getCharPos(inMaxGraphemes - 1);
        QTextCharFormat fmt;
        fmt.setFont(document()->defaultFont());
        setFormat(0, charPos, fmt);

        const auto exceededLen = text.length() - charPos;
        setFormat(charPos, exceededLen, mLengthExceededFormat);
    }
}

int EmojiFixHighlighter::getPrevBlockTotalCharLength() const
{
    if (currentBlock().blockNumber() == -1)
        return -1;

    if (currentBlock().blockNumber() == 0)
        return 0;

    const int blockState = previousBlockState();

    if (blockState == -1)
        return -1;

    const int prevCharLength = (blockState >> 16) & 0x0000ffff;
    return prevCharLength + 1; // +1 for newline
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

            if (UnicodeFonts::isEmoji(grapheme))
                addFormat(prev, len, mEmojiFormat);
        }

        prev = next;
    }
}

}
