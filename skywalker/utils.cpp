// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "utils.h"
#include "android_utils.h"
#include "QDebug"

namespace Skywalker {

bool Utils::sEmojiPickerShown = false;

std::optional<QString> Utils::makeOptionalString(const QString& str)
{
    std::optional<QString> optionalString;
    if (!str.isEmpty())
        optionalString = str;

    return optionalString;
}

QColor Utils::determineForegroundColor(const QColor& backgroundColor, const QColor& lightColor, const QColor& darkColor)
{
    const float blackness = backgroundColor.toCmyk().blackF();
    qDebug() << "Blackness:" << blackness;
    return blackness < 0.3 ? lightColor : darkColor;
}

bool Utils::similarColors(const QColor& lhs, const QColor& rhs)
{
    return std::abs(lhs.red() - rhs.red()) < 32 && std::abs(lhs.green() - rhs.green()) < 32 && std::abs(lhs.blue() - rhs.blue()) < 32;
}

bool Utils::translate(const QString& text)
{
    return AndroidUtils::translate(text);
}

void Utils::showEmojiPicker()
{
    if (!sEmojiPickerShown)
    {
        AndroidUtils::showEmojiPicker();
        sEmojiPickerShown = true;
    }
}

void Utils::dismissEmojiPicker()
{
    if (sEmojiPickerShown)
    {
        AndroidUtils::dismissEmojiPicker();
        sEmojiPickerShown = false;
    }
}

}
