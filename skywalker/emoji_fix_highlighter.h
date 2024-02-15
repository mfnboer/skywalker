// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QSyntaxHighlighter>

namespace Skywalker {

class EmojiFixHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit EmojiFixHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    void setEmojiFontKeycaps(const QString& text);
    void setEmojiFontCombinedEmojis(const QString& text);
};

}
