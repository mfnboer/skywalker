// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_list_model.h"
#include "favorite_feeds.h"

namespace Skywalker {

ListListModel::ListListModel(Type type, const QString& atId, const FavoriteFeeds& favoriteFeeds, QObject* parent) :
    QAbstractListModel(parent),
    mType(type),
    mAtId(atId),
    mFavoriteFeeds(favoriteFeeds)
{
    qDebug() << "New list list model type:" << type << "atId:" << atId;
    connect(&mFavoriteFeeds, &FavoriteFeeds::listSaved, this, [this]{ listSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listPinned, this, [this]{ listPinnedChanged(); });
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
    case Role::ListSaved:
        return mFavoriteFeeds.isSavedFeed(list.getUri());
    case Role::ListPinned:
        return mFavoriteFeeds.isPinnedFeed(list.getUri());
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

void ListListModel::addLists(const QList<ListView>& lists)
{
    qDebug() << "Add lists:" << lists.size();

    if (lists.empty())
    {
        qDebug() << "No new lists";
        return;
    }

    const size_t newRowCount = mLists.size() + lists.size();

    beginInsertRows({}, mLists.size(), newRowCount - 1);
    mLists.insert(mLists.end(), lists.begin(), lists.end());
    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
}

void ListListModel::prependList(const ListView& list)
{
    qDebug() << "Prepend list:" << list.getName();

    beginInsertRows({}, 0, 0);
    mLists.push_front(list);
    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
}

void ListListModel::updateEntry(int index, const QString& cid, const QString& name, const QString& description, const QString& avatar)
{
    qDebug() << "Update entry:" << name << "index:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return;
    }

    auto& list = mLists[index];

    if (cid != list.getCid())
        list.setCid(cid);

    if (name != list.getName())
        list.setName(name);

    if (description != list.getDescription())
        list.setDescription(description);

    if (avatar != list.getAvatar())
        list.setAvatar(avatar);

    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
}

void ListListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return;
    }

    beginRemoveRows({}, index, index);
    mLists.erase(mLists.begin() + index);
    endRemoveRows();
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
        { int(Role::ListCreator), "listCreator" },
        { int(Role::ListSaved), "listSaved" },
        { int(Role::ListPinned), "listPinned" }
    };

    return roles;
}

void ListListModel::listSavedChanged()
{
    changeData({ int(Role::ListSaved) });
}

void ListListModel::listPinnedChanged()
{
    changeData({ int(Role::ListPinned) });
}

void ListListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mLists.size() - 1, 0), roles);
}

}
