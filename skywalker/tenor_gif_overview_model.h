// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "tenor_preview_row.h"
#include <QAbstractListModel>

namespace Skywalker {

class TenorOverviewModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Role {
        PreviewRow = Qt::UserRole + 1,
        PreviewRowSpacing
    };

    TenorOverviewModel(int maxRowWidth, int spacing, QObject* parent = nullptr);

    void setMaxRowWidth(int width) { mMaxRowWidth = width; }
    void setSpacing(int spacing) { mSpacing = spacing; }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addGifs(const TenorGifList& gifs);

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    void addRow(const TenorPreviewRow& row);

    int mMaxRowWidth;
    int mSpacing;
    std::vector<TenorPreviewRow> mOverview;
};

}
