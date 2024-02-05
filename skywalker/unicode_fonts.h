// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <QtQmlIntegration>
#include <QObject>

namespace Skywalker {

class UnicodeFonts : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    using FontType = QEnums::FontType;

    static bool isAlpha(QChar c);
    static bool isDigit(QChar c);

    // Returns 0 if there is no conversion
    static uint convertToFont(QChar c, FontType font);

    static bool convertLastCharsToFont(QString& text, int numChars, FontType font);

    static QString toPlainText(const QString& text);
    Q_INVOKABLE static QString normalizeToNFKD(const QString& text);
    Q_INVOKABLE static int graphemeLength(const QString& text);
    Q_INVOKABLE static bool onlyEmojis(const QString& text);

    static bool isEmoji(uint c);

private:
    static uint convertToSmallCaps(QChar c);
};

}
