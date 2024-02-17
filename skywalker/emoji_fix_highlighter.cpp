// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "emoji_fix_highlighter.h"
#include "font_downloader.h"
#include "unicode_fonts.h"

namespace Skywalker {

EmojiFixHighlighter::EmojiFixHighlighter(QTextDocument* parent) :
    QSyntaxHighlighter(parent)
{
}

void EmojiFixHighlighter::setMaxLength(int maxLength, const QString& lengthExceededColor)
{
    mMaxLength = maxLength;
    mLengthExceededColor = lengthExceededColor;
}

void EmojiFixHighlighter::highlightBlock(const QString& text)
{
    setEmojiFontKeycaps(text);
    setEmojiFontCombinedEmojis(text);
    highlightLengthExceeded(text);
}

void EmojiFixHighlighter::highlightLengthExceeded(const QString&)
{
    // TODO
}

void EmojiFixHighlighter::setEmojiFontKeycaps(const QString& text)
{
    static const QRegularExpression enclosingKeycapRE("(.\uFE0F\u20E3)");
    auto i = text.indexOf(enclosingKeycapRE);

    while (i != -1)
    {
        setFormat(i, 3, FontDownloader::getEmojiFont());
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
                setFormat(prev, len, FontDownloader::getEmojiFont());
        }

        prev = next;
    }
}

}
