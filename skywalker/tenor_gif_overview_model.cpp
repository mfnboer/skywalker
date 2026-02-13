// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "tenor_gif_overview_model.h"

namespace Skywalker {

TenorOverviewModel::TenorOverviewModel(int maxRowWidth, int spacing, QObject* parent) :
    QAbstractListModel(parent),
    mMaxRowWidth(maxRowWidth),
    mSpacing(spacing)
{
}

void TenorOverviewModel::setMaxRowWidth(int width)
{
    if (mMaxRowWidth == width)
        return;

    mMaxRowWidth = width;
    QTimer::singleShot(0, this, [this]{ refresh(); });
}

void TenorOverviewModel::setSpacing(int spacing)
{
    if (mSpacing == spacing)
        return;

    mSpacing = spacing;
    QTimer::singleShot(0, this, [this]{ refresh(); });
}

void TenorOverviewModel::clear()
{
    if (mOverview.empty())
        return;

    beginRemoveRows({}, 0, mOverview.size() - 1);
    mOverview.clear();
    mGifs.clear();
    endRemoveRows();
}

void TenorOverviewModel::addGifs(const TenorGifList& gifs)
{
    for (const auto& gif : gifs)
        mGifs.append(gif.deepCopy());

    TenorPreviewRow nextRow(mMaxRowWidth, mSpacing);

    if (!mOverview.empty() && !mOverview.back().isFull())
    {
        const auto newSize = mOverview.size() - 1;
        nextRow = mOverview.back();
        beginRemoveRows({}, newSize, newSize);
        mOverview.erase(mOverview.begin() + mOverview.size() - 1);
        endRemoveRows();
    }

    for (const auto& gif : gifs)
    {
        qDebug() << "Adding GIF:" << gif.getDescription();
        nextRow.addGif(gif);

        if (nextRow.isFull())
        {
            addRow(nextRow);
            nextRow = TenorPreviewRow(mMaxRowWidth, mSpacing);
        }
    }

    if (!nextRow.isEmpty())
        addRow(nextRow);
}

void TenorOverviewModel::addRow(const TenorPreviewRow& row)
{
    qDebug() << "Adding row:" << row.getRow().size();
    beginInsertRows({}, mOverview.size(), mOverview.size());
    mOverview.push_back(row);
    endInsertRows();
}

void TenorOverviewModel::refresh()
{
    if (mGifs.empty())
        return;

    const auto gifs = mGifs;
    clear();
    addGifs(gifs);
}

int TenorOverviewModel::rowCount(const QModelIndex&) const
{
    return mOverview.size();
}

QVariant TenorOverviewModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mOverview.size())
        return {};

    const auto& previewRow = mOverview[index.row()];

    switch (Role(role))
    {
    case Role::PreviewRow:
        return QVariant::fromValue(previewRow.getRow());
    case Role::PreviewRowSpacing:
        return previewRow.getSpacing();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

QHash<int, QByteArray> TenorOverviewModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::PreviewRow), "previewRow" },
        { int(Role::PreviewRowSpacing), "previewRowSpacing" }
    };

    return roles;
}

}
