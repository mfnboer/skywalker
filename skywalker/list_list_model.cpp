// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_list_model.h"
#include "skywalker.h"
#include "favorite_feeds.h"

namespace Skywalker {

ListListModel::ListListModel(Type type, Purpose purpose, const QString& atId,
                             const FavoriteFeeds& favoriteFeeds, Skywalker* skywalker,
                             QObject* parent) :
    QAbstractListModel(parent),
    mType(type),
    mPurpose(purpose),
    mAtId(atId),
    mFavoriteFeeds(favoriteFeeds),
    mGraphUtils(this)
{
    qDebug() << "New list list model type:" << type << "purpose:" << purpose << "atId:" << atId;
    mGraphUtils.setSkywalker(skywalker);

    connect(&mFavoriteFeeds, &FavoriteFeeds::listSaved, this, [this]{ listSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listPinned, this, [this]{ listPinnedChanged(); });

    connect(&mGraphUtils, &GraphUtils::isListUserOk, this,
            [this](const QString& listUri, const QString&, const QString& listItemUri){
                updateMemberCheckResults(listUri, listItemUri);
            });
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
    const auto* change = getLocalChange(list.getUri());

    switch (Role(role))
    {
    case Role::List:
        return QVariant::fromValue(list);
    case Role::ListCreator:
        return QVariant::fromValue(list.getCreator());
    case Role::ListBlockedUri:
        return change && change->mBlocked ? *change->mBlocked : list.getViewer().getBlocked();
    case Role::ListMuted:
        return change && change->mMuted ? *change->mMuted : list.getViewer().getMuted();
    case Role::ListSaved:
        return mFavoriteFeeds.isSavedFeed(list.getUri());
    case Role::ListPinned:
        return mFavoriteFeeds.isPinnedFeed(list.getUri());
    case Role::MemberCheck:
        if (change && change->mMemberListItemUri)
            return change->mMemberListItemUri->isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;

        return memberCheck(list.getUri());
    case Role::MemberListItemUri:
        return change && change->mMemberListItemUri ? *change->mMemberListItemUri : getMemberListItemUri(list.getUri());
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

    mMemberCheckResults.clear();
    mCursor.clear();
    clearLocalChanges();
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

    if (!mMemberCheckDid.isEmpty())
    {
        for (const auto& l : filteredLists)
            mGraphUtils.isListUser(l.getUri(), mMemberCheckDid);
    }

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

    if (!mMemberCheckDid.isEmpty())
    {
        for (const auto& l : lists)
            mGraphUtils.isListUser(l.getUri(), mMemberCheckDid);
    }

    qDebug() << "New lists size:" << mLists.size();
}

void ListListModel::prependList(const ListView& list)
{
    qDebug() << "Prepend list:" << list.getName();

    beginInsertRows({}, 0, 0);
    mLists.push_front(list);
    endInsertRows();

    if (!mMemberCheckDid.isEmpty())
        mGraphUtils.isListUser(list.getUri(), mMemberCheckDid);

    qDebug() << "New lists size:" << mLists.size();
}

ListView ListListModel::updateEntry(int index, const QString& cid, const QString& name, const QString& description, const QString& avatar)
{
    qDebug() << "Update entry:" << name << "index:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return {};
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
    return list;
}

void ListListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return;
    }

    const QString listUri = mLists[index].getUri();

    beginRemoveRows({}, index, index);
    mLists.erase(mLists.begin() + index);
    endRemoveRows();

    if (!mMemberCheckDid.isEmpty())
        mMemberCheckResults.erase(listUri);
}

ListView ListListModel::getEntry(int index) const
{
    qDebug() << "Get entry:" << index;

    if (index < 0 || (size_t)index >= mLists.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mLists.size();
        return {};
    }

    return mLists[index];
}

ListListModel::ListList ListListModel::filterLists(ATProto::AppBskyGraph::ListViewList lists) const
{
    ListList filtered;

    for (auto&& listView : lists)
    {
        if (mPurpose == Purpose::LIST_PURPOSE_UNKNOWN ||
            listView->mPurpose == ATProto::AppBskyGraph::ListPurpose(mPurpose))
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
        { int(Role::ListBlockedUri), "listBlockedUri" },
        { int(Role::ListMuted), "listMuted" },
        { int(Role::ListSaved), "listSaved" },
        { int(Role::ListPinned), "listPinned" },
        { int(Role::MemberCheck), "memberCheck" },
        { int(Role::MemberListItemUri), "memberListItemUri" }
    };

    return roles;
}

QEnums::TripleBool ListListModel::memberCheck(const QString& listUri) const
{
    if (mMemberCheckDid.isEmpty())
        return QEnums::TRIPLE_BOOL_UNKNOWN;

    auto it = mMemberCheckResults.find(listUri);

    if (it != mMemberCheckResults.end())
    {
        const std::optional<QString>& listItemUri = it->second;

        if (!listItemUri)
            return QEnums::TRIPLE_BOOL_UNKNOWN;

        return listItemUri->isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;
    }

    return QEnums::TRIPLE_BOOL_UNKNOWN;
}

void ListListModel::updateMemberCheckResults(const QString& listUri, const QString& listItemUri)
{
    qDebug() << "List:" << listUri << "Item:" << listItemUri;
    mMemberCheckResults[listUri] = listItemUri;
    changeData({ int(Role::MemberCheck), int(Role::MemberListItemUri) });
}

QString ListListModel::getMemberListItemUri(const QString& listUri) const
{
    qDebug() << "Get member, list:" << listUri;

    if (mMemberCheckDid.isEmpty())
        return {};

    auto it = mMemberCheckResults.find(listUri);

    if (it == mMemberCheckResults.end())
        return {};

    qDebug() << "Get member, list:" << listUri << "item:" << *(it->second);
    return *(it->second);
}

void ListListModel::blockedChanged()
{
    changeData({ int(Role::ListBlockedUri) });
}

void ListListModel::mutedChanged()
{
    changeData({ int(Role::ListMuted) });
}

void ListListModel::memberListItemUriChanged()
{
    changeData({ int(Role::MemberCheck), int(Role::MemberListItemUri) });
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
