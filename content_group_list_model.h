// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "content_filter.h"
#include <QAbstractListModel>

namespace Skywalker {

class ContentGroupListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        ContentGroup = Qt::UserRole + 1,
        ContentVisibility
    };

    explicit ContentGroupListModel(const ContentFilter& contentFilter, QObject* parent = nullptr);
    void initContentGroups();
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    const ContentFilter& mContentFilter;
    std::vector<ContentGroup> mContentGroupList;
};

}
