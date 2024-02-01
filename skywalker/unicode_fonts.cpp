// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "unicode_fonts.h"
#include <unordered_map>

namespace Skywalker {

namespace {
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
        { QEnums::FONT_BUBBLE, { 0x24B6, 0x24D0, 0x2459 } }, // NOTE: 0 = 0x24EA
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

}
