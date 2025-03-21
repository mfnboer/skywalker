// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "emoji_fix_highlighter.h"
#include "web_link.h"
#include <atproto/lib/rich_text_master.h>

namespace Skywalker {

class FacetHighlighter : public EmojiFixHighlighter
{
    Q_OBJECT

public:
    explicit FacetHighlighter(QTextDocument* parent = nullptr);
    void setHighlightColor(const QString& colorName);
    void setEmbeddedLinks(const WebLink::List* links);

protected:
    void highlightBlock(const QString& text) override;

private:
    bool facetOverlapsWithEmbeddedLink(const ATProto::RichTextMaster::ParsedMatch& facet) const;
    void highlightEmbeddedLinks();

    QTextCharFormat mHighlightFormat;
    const WebLink::List* mEmbeddedLinks = nullptr;
};

}
