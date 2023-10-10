// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "content_group_list_model.h"

namespace Skywalker {

ContentGroupListModel::ContentGroupListModel(const ContentFilter& contentFilter, QObject* parent) :
    QAbstractListModel(parent),
    mContentFilter(contentFilter)
{
    initContentGroups();
}

void ContentGroupListModel::initContentGroups()
{
    clear();

    beginInsertRows({}, 0, ContentFilter::CONTENT_GROUPS.size() - 1);

    for (const auto& [_, group] : ContentFilter::CONTENT_GROUPS)
        mContentGroupList.push_back(group);

    endInsertRows();
}

void ContentGroupListModel::clear()
{
    beginRemoveRows({}, 0, mContentGroupList.size() - 1);
    mContentGroupList.clear();
    endRemoveRows();
}

int ContentGroupListModel::rowCount(const QModelIndex&) const
{
    return mContentGroupList.size();
}

QVariant ContentGroupListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= mContentGroupList.size())
        return {};

    const auto& group = mContentGroupList[index.row()];

    switch (Role(role))
    {
    case Role::ContentGroup:
        return QVariant::fromValue(group);
    case Role::ContentVisibility:
        return mContentFilter.getVisibility(group.mId);
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

QHash<int, QByteArray> ContentGroupListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::ContentGroup), "contentGroup" },
        { int(Role::ContentVisibility), "contentVisibility" }
    };

    return roles;
}

}
