// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_list_model.h"

namespace Skywalker {

ListListModel::ListListModel(Type type, const QString& atId, QObject* parent) :
    QAbstractListModel(parent),
    mType(type),
    mAtId(atId)
{
    qDebug() << "New list list model type:" << type << "atId:" << atId;
}

int ListListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mLists.size();
}

QVariant ListListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mLists.size())
        return {};

    const auto& list = mLists[index.row()];

    switch (Role(role))
    {
    case Role::List:
        return QVariant::fromValue(list);
    case Role::ListCreator:
        return QVariant::fromValue(list.getCreator());
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void ListListModel::clear()
{
    qDebug() << "Clear lists";

    if (!mLists.empty())
    {
        beginRemoveRows({}, 0, mLists.size() - 1);
        mLists.clear();
        endRemoveRows();
    }

    mCursor.clear();
}

int ListListModel::addLists(ATProto::AppBskyGraph::ListViewList lists, const QString& cursor)
{
    qDebug() << "Add lists:" << lists.size() << "cursor:" << cursor;
    mCursor = cursor;

    const auto filteredLists = filterLists(std::move(lists));

    if (filteredLists.empty())
    {
        qDebug() << "No new lists";
        return 0;
    }

    const size_t newRowCount = mLists.size() + filteredLists.size();

    beginInsertRows({}, mLists.size(), newRowCount - 1);
    mLists.insert(mLists.end(), filteredLists.begin(), filteredLists.end());
    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
    return filteredLists.size();
}

void ListListModel::prependList(const ListView& list)
{
    qDebug() << "Prepend list:" << list.getName();

    beginInsertRows({}, 0, 0);
    mLists.push_front(list);
    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
}

void ListListModel::updateEntry(int index, const ListView& list)
{
    qDebug() << "Update entry:" << list.getName() << "index:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return;
    }

    mLists[index] = list;
    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
}

ListListModel::ListList ListListModel::filterLists(ATProto::AppBskyGraph::ListViewList lists) const
{
    ListList filtered;

    for (auto&& listView : lists)
    {
        if (listView->mPurpose == ATProto::AppBskyGraph::ListPurpose(mType))
        {
            ATProto::AppBskyGraph::ListView::SharedPtr sharedRaw(listView.release());
            filtered.emplace_back(sharedRaw);
        }
    }

    return filtered;
}

QHash<int, QByteArray> ListListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::List), "list" },
        { int(Role::ListCreator), "listCreator" }
    };

    return roles;
}

}
