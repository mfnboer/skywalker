// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QSyntaxHighlighter>

namespace Skywalker {

class FacetHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit FacetHighlighter(QTextDocument* parent = nullptr);
    void setHighlightColor(const QString& colorName);

protected:
    void highlightBlock(const QString& text) override;
    void setEmojiFontKeycaps(const QString& text);
    void setEmojiFontCombinedEmojis(const QString& text);

private:
    QColor mHighlightColor;
};

}
