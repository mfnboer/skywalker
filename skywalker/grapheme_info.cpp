// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "grapheme_info.h"

namespace Skywalker {

GraphemeInfo::GraphemeInfo(int length, const std::vector<int>& charPositions) :
    mLength(length),
    mCharPositions(charPositions)
{
}

int GraphemeInfo::getCharPos(int graphemeIndex) const
{
    Q_ASSERT(graphemeIndex >= 0);
    Q_ASSERT(graphemeIndex <= (int)mCharPositions.size());

    if (graphemeIndex < 0 || graphemeIndex >= (int)mCharPositions.size())
        return -1;

    return mCharPositions.at(graphemeIndex);
}

QString GraphemeInfo::sliced(const QString& text, int startIndex, int sz)
{
    const int startPos = getCharPos(startIndex);
    const int endPos = (startIndex + sz >= getLength()) ? text.length() : getCharPos(startIndex + sz);
    const int len = endPos - startPos;
    return text.sliced(startPos, len);
}

QString GraphemeInfo::sliced(const QString& text, int startIndex)
{
    return sliced(text, startIndex, getLength());
}

}
