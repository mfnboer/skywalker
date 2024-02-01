// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <QtQmlIntegration>
#include <QObject>

namespace Skywalker {

class UnicodeFonts
{
public:
    using FontType = QEnums::FontType;

    static bool isAlpha(QChar c);
    static bool isDigit(QChar c);

    // Returns 0 if there is no conversion
    static uint convertToFont(QChar c, FontType font);

    static bool convertLastCharToFont(QString& text, FontType font);
};

}
