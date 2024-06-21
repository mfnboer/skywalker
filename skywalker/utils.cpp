// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "utils.h"
#include "QDebug"

namespace Skywalker {

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

}
