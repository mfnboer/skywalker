// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QQmlEngine>

namespace Skywalker {

class WordsHighlighter : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit WordsHighlighter(QObject* parent = nullptr);

    QString highlight(const QString& text, const QString& words, const QString& color) const;

private:
};

}
