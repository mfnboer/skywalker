// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QColor>
#include <QDateTime>
#include <QString>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class Utils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    static std::optional<QString> makeOptionalString(const QString& str);
    Q_INVOKABLE static QColor determineForegroundColor(const QColor& background, const QColor& lightColor, const QColor& darkColor);
    Q_INVOKABLE static bool similarColors(const QColor& lhs, const QColor& rhs);
    Q_INVOKABLE static bool translate(const QString& text);
    Q_INVOKABLE static void showEmojiPicker();
    Q_INVOKABLE static void dismissEmojiPicker();
    Q_INVOKABLE static bool isEmojiPickerShown() { return sEmojiPickerShown; }

private:
    static bool sEmojiPickerShown;
};

}
