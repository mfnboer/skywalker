// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "unicode_fonts.h"
#include <unordered_map>

namespace Skywalker {

namespace {

constexpr char const* COMBINING_LONG_STROKE_OVERLAY = "\u0336";

struct FontCodePoint
{
    uint mUpperA;
    uint mLowerA;
    uint mDigit0;
};
}

bool UnicodeFonts::isAlpha(QChar c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool UnicodeFonts::isDigit(QChar c)
{
    return (c >= '0' && c <= '9');
}

uint UnicodeFonts::convertToFont(QChar c, FontType font)
{
    static std::unordered_map<FontType, FontCodePoint> FONT_CODE_POINTS = {
        { QEnums::FONT_BOLD, { 0x1D5D4, 0x1D5EE, 0x1D7EC } },
        { QEnums::FONT_ITALIC, { 0x1D608, 0x1D622, 0 } },
        { QEnums::FONT_MONOSPACE, { 0x1D670, 0x1D68A, 0x1D7F6 } },
        { QEnums::FONT_CURSIVE, { 0x1D4D0, 0x1D4EA, 0 } },
        { QEnums::FONT_FULLWIDTH, { 0xFF21, 0xFF41, 0xFF10 } },
        { QEnums::FONT_BUBBLE, { 0x24B6, 0x24D0, 0x245F } }, // NOTE: 0 = 0x24EA
        { QEnums::FONT_SQUARE, { 0x1F130, 0x1F130, 0 } } // NOTE: only upper case
    };

    if (font == QEnums::FONT_NORMAL)
        return {};

    if (!isAlpha(c) && !isDigit(c))
        return 0;

    auto it = FONT_CODE_POINTS.find(font);
    Q_ASSERT(it != FONT_CODE_POINTS.end());

    if (it == FONT_CODE_POINTS.end())
    {
        qWarning() << "Undefined font type:" << font;
        return 0;
    }

    const auto& fontCodePoint = it->second;

    if (isDigit(c))
    {
        if (!fontCodePoint.mDigit0)
            return 0;

        if (font == QEnums::FONT_BUBBLE && c == '0')
            return 0x24EA;

        return c.unicode() - '0' + fontCodePoint.mDigit0;
    }

    if (c.isUpper())
        return c.unicode() - 'A' + fontCodePoint.mUpperA;
    else
        return c.unicode() - 'a' + fontCodePoint.mLowerA;
}

bool UnicodeFonts::convertLastCharsToFont(QString& text, int numChars, FontType font)
{
    if (font == QEnums::FONT_NORMAL)
        return false;

    if (text.isEmpty())
        return false;

    if (numChars < 1 || numChars > text.length())
        return false;

    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    boundaryFinder.toEnd();
    auto convertBoundary = boundaryFinder.position();
    QString converted;
    bool conversionDone = false;

    for (int i = 0; i < numChars; ++i)
    {
        const auto lastBoundary = convertBoundary;
        convertBoundary = boundaryFinder.toPreviousBoundary();
        const auto graphemeLen = lastBoundary - convertBoundary;

        if (convertBoundary == -1)
        {
            qWarning() << "No previous grapheme boundary:" << i << "text:" << text;
            return false;
        }

        QString grapheme = text.sliced(convertBoundary, graphemeLen);

        if (font == QEnums::FONT_STRIKETHROUGH)
        {
            converted.push_front(COMBINING_LONG_STROKE_OVERLAY);
            converted.push_front(grapheme);
            conversionDone = true;
        }
        else if (graphemeLen == 1)
        {
            const auto ch = grapheme.back();
            const char32_t convertedUcs4 = convertToFont(ch, font);

            if (convertedUcs4)
            {
                converted.push_front(QString::fromUcs4(&convertedUcs4, 1));
                conversionDone = true;
            }
            else
            {
                converted.push_front(grapheme);
            }
        }
    }

    if (!conversionDone)
    {
        qDebug() << "Nothing to convert, font:" << font << "num:" << numChars << "text:" << text;
        return false;
    }

    text.chop(text.size() - convertBoundary);
    text += converted;
    return true;
}

}
