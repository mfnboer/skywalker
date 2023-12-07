// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "tenor_gif.h"

namespace Skywalker {

class TenorPreviewRow
{
public:
    TenorPreviewRow(int maxRowWidth, int spacing);

    const TenorGifList& getRow() const { return mRow; }
    int getSpacing() const { return mSpacing; }

    bool isEmpty() const { return mRow.empty(); }
    bool isFull() const;
    int getTotalWidth() const;
    int getTotalSpacing() const;
    bool addGif(TenorGif gif);

private:
    void scaleRowToWidth(int newWidth);
    void scaleRowToHeight(int newHeight);
    void scaleGifToHeight(TenorGif& gif, int newHeight) const;

    int mMaxRowWidth;
    int mSpacing;
    TenorGifList mRow;
    int mRowWidth = 0; // without spacing
    int mRowHeight = 1000;
};

}
