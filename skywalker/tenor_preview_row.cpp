// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor_preview_row.h"

namespace Skywalker {

TenorPreviewRow::TenorPreviewRow(int maxRowWidth, int spacing) :
    mMaxRowWidth(maxRowWidth),
    mSpacing(spacing)
{
}

bool TenorPreviewRow::isFull() const
{
    return getTotalWidth() >= mMaxRowWidth;
}

int TenorPreviewRow::getTotalWidth() const
{
    return mRowWidth + getTotalSpacing();
}

int TenorPreviewRow::getTotalSpacing() const
{
    if (mRow.isEmpty())
        return 0;

    return (mRow.size() - 1) * mSpacing;
}

bool TenorPreviewRow::addGif(TenorGif gif)
{
    Q_ASSERT(!isFull());

    if (isFull())
    {
        qWarning() << "Row is already full:" << mRow.size();
        return false;
    }

    if (gif.getOverviewSize().height() < mRowHeight)
        scaleRowToHeight(gif.getOverviewSize().height());
    else if (gif.getOverviewSize().height() > mRowHeight)
        scaleGifToHeight(gif, mRowHeight);

    mRow.append(gif);
    mRowWidth += gif.getOverviewSize().width();

    if (!isFull())
        return true;

    if (getTotalWidth() == mMaxRowWidth)
        return true;

    scaleRowToWidth(mMaxRowWidth - getTotalSpacing());
    return true;
}

void TenorPreviewRow::scaleRowToWidth(int newWidth)
{
    const qreal scale = qreal(newWidth) / mRowWidth;
    const int newHeight = mRowHeight * scale;
    scaleRowToHeight(newHeight);

    if (mRowWidth != newWidth)
    {
        // Can happen due to rounding
        auto& gif = mRow.last();
        auto size = gif.getOverviewSize();
        size.rwidth() += newWidth - mRowWidth;
        gif.setOverviewSize(size);
        mRowWidth = newWidth;
    }
}

void TenorPreviewRow::scaleRowToHeight(int newHeight)
{
    mRowWidth = 0;

    for (auto& gif : mRow)
    {
        scaleGifToHeight(gif, newHeight);
        mRowWidth += gif.getOverviewSize().width();
    }

    mRowHeight = newHeight;
}

void TenorPreviewRow::scaleGifToHeight(TenorGif& gif, int newHeight) const
{
    const qreal scale = qreal(newHeight) / gif.getOverviewSize().height();
    QSize newSize = gif.getOverviewSize() * scale;
    newSize.setHeight(newHeight); // to avoid rounding error
    gif.setOverviewSize(newSize);
}

}
