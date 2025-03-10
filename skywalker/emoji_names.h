// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class EmojiNames : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_INVOKABLE static QString getEmojiName(const QString emoji);

private:
    static const std::unordered_map<QString, QString> EMOJI_NAMES_MAP;
};

}
