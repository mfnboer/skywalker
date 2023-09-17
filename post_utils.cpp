// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_utils.h"

namespace Skywalker {

PostUtils::PostUtils(QObject* parent) :
    QObject(parent)
{
}

void PostUtils::setSkywalker(Skywalker* skywalker)
{
    mSkywalker = skywalker;
    emit skywalkerChanged();
}

QString PostUtils::highlightMentionsAndLinks(const QString& text)
{
    const auto facets = mSkywalker->getBskyClient()->parseFacets(text);
    QString highlighted;
    int pos = 0;

    for (const auto& facet : facets)
    {
        const auto before = text.sliced(pos, facet.mStartIndex - pos);
        highlighted.append(before.toHtmlEscaped().replace(' ', "&nbsp;"));
        QString highlight = QString("<font color=\"blue\">%1</font>").arg(facet.mMatch);
        highlighted.append(highlight);
        pos = facet.mEndIndex;
    }

    highlighted.append(text.sliced(pos).toHtmlEscaped().replace(' ', "&nbsp;"));
    return highlighted;
}

int PostUtils::graphemeLength(const QString& text)
{
    QTextBoundaryFinder boundaryFinder(QTextBoundaryFinder::Grapheme, text);
    int length = 0;

    while (boundaryFinder.toNextBoundary() != -1)
        ++length;

    return length;
}

}
