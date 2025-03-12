// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "grapheme_info.h"
#include <QQuickTextDocument>
#include <qqmlintegration.h>
#include <QObject>

namespace Skywalker {

class UnicodeFonts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    using FontType = QEnums::FontType;

    static bool isAlpha(QChar c);
    static bool isDigit(QChar c);

    // Returns 0 if there is no conversion
    static uint convertToFont(QChar c, FontType font);

    static bool convertLastCharsToFont(QString& text, int numChars, FontType font);

    Q_INVOKABLE static QString getEmojiFontFamily();
    Q_INVOKABLE static QString getEmojiFontSource();
    Q_INVOKABLE static QString toPlainText(const QString& text);
    Q_INVOKABLE static QString toCleanedHtml(const QString& text);
    Q_INVOKABLE static QString normalizeToNFKD(const QString& text);
    Q_INVOKABLE static int graphemeLength(const QString& text);
    Q_INVOKABLE static GraphemeInfo getGraphemeInfo(const QString& text);
    Q_INVOKABLE static bool onlyEmojis(const QString& text);
    Q_INVOKABLE static bool hasEmoji(const QString& text);
    Q_INVOKABLE static bool hasCombinedEmojis(const QString& text);
    Q_INVOKABLE static QStringList getUniqueEmojis(const QString& text);
    Q_INVOKABLE static bool isHashtag(const QString& text);
    Q_INVOKABLE static QStringList splitText(const QString& text, int maxLength, int minSplitLineLength, int maxParts = 1000000);

    static bool isEmoji(uint c); // heuristic
    static bool isKeycapEmoji(const QString& grapheme);
    static QString setEmojiFontCombinedEmojis(const QString& text);

private:
    static uint convertToSmallCaps(QChar c);
};

}
