// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>
#include <QtQmlIntegration>

namespace Skywalker {

class HighlightState : public QTextBlockUserData
{
public:
    int mTextLength = 0; // in characters
};

class EmojiFixHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit EmojiFixHighlighter(QTextDocument* parent = nullptr);
    void setMaxLength(int maxLength, const QString& lengthExceededColor);
    void setMaxLength(int maxLength);

    Q_INVOKABLE void setEmojiFixDocument(QQuickTextDocument* doc, int maxLength = -1, const QString& lengthExceededColor = {});

protected:
    void highlightBlock(const QString& text) override;
    int getPrevBlockTotalCharLength() const;
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
