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
    void setMaxLength(int maxLength, const QString& lengthExceededColor);
    void setMaxLength(int maxLength);

protected:
    void highlightBlock(const QString& text) override;
    void addFormat(int start, int sz, const QTextCharFormat& fmt);

private:
    void highlightLengthExceeded(const QString& text);
    void setEmojiFontKeycaps(const QString& text);
    void setEmojiFontCombinedEmojis(const QString& text);

    int mMaxLength = -1;
    QTextCharFormat mLengthExceededFormat;
    QTextCharFormat mEmojiFormat;
};

}
