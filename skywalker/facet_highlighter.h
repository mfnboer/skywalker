// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "emoji_fix_highlighter.h"

namespace Skywalker {

class FacetHighlighter : public EmojiFixHighlighter
{
    Q_OBJECT

public:
    explicit FacetHighlighter(QTextDocument* parent = nullptr);
    void setHighlightColor(const QString& colorName);

protected:
    void highlightBlock(const QString& text) override;

private:
    QColor mHighlightColor;
};

}
