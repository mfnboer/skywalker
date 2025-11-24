// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "wrapped_skywalker.h"
#include <QColor>
#include <QDateTime>

namespace Skywalker {

class Utils : public WrappedSkywalker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Utils(QObject* parent = nullptr);

    static std::optional<QString> makeOptionalString(const QString& str);
    Q_INVOKABLE static QColor determineForegroundColor(const QColor& background, const QColor& lightColor, const QColor& darkColor);
    Q_INVOKABLE static bool similarColors(const QColor& lhs, const QColor& rhs);
    Q_INVOKABLE static bool translate(const QString& text);
    Q_INVOKABLE void showEmojiPicker();
    Q_INVOKABLE static void dismissEmojiPicker();
    Q_INVOKABLE static bool isEmojiPickerShown() { return sEmojiPickerShown; }
    Q_INVOKABLE static bool isFollowingListUri(const QString& uri);
    Q_INVOKABLE static QString getFollowingUri();

signals:
    void emojiPicked(QString emoji);

private:
    void handleEmojiPicked(const QString& emoji);

    static bool sEmojiPickerShown;
};

}
