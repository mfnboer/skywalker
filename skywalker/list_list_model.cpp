// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "list_list_model.h"
#include "graph_utils.h"
#include "skywalker.h"
#include "favorite_feeds.h"
#include "search_utils.h"

namespace Skywalker {

ListListModel::ListListModel(Type type, Purpose purpose, const QString& atId,
                             const FavoriteFeeds& favoriteFeeds, Skywalker* skywalker,
                             QObject* parent) :
    BaseType(skywalker->getUserDid(), parent),
    mType(type),
    mPurpose(purpose),
    mAtId(atId),
    mFavoriteFeeds(favoriteFeeds),
    mTimelineHide(*skywalker->getTimelineHide()),
    mUserSettings(*skywalker->getUserSettings())
{
    qDebug() << "New list list model type:" << type << "purpose:" << purpose << "atId:" << atId;

    connect(&mFavoriteFeeds, &FavoriteFeeds::listSaved, this, [this]{ listSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listPinned, this, [this]{ listPinnedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listUnpinned, this, [this](QString){ listPinnedChanged(); });
}

QVariant ListListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& list = mList[index.row()];
    const auto* change = getLocalChange(list.getUri());

    switch (Role(role))
    {
    case Role::List:
        return QVariant::fromValue(list);
    case Role::ListUri:
        return list.getUri();
    case Role::ListName:
        return list.getName();
    case Role::ListCreator:
    {
        auto creator = list.getCreator();
        const Profile* profileChange = getProfileChange(creator.getDid());
        return QVariant::fromValue(profileChange ? *profileChange : creator);
    }
    case Role::ListBlockedUri:
        return change && change->mBlocked ? *change->mBlocked : list.getViewer().getBlocked();
    case Role::ListMuted:
        return change && change->mMuted ? *change->mMuted : list.getViewer().getMuted();
    case Role::ListSaved:
        return mFavoriteFeeds.isSavedFeed(list.getUri());
    case Role::ListPinned:
        return mFavoriteFeeds.isPinnedFeed(list.getUri());
    case Role::ListHideFromTimeline:
        return change &&change->mHideFromTimeline ? *change->mHideFromTimeline : mTimelineHide.hasList(list.getUri());
    case Role::ListSync:
        return mUserSettings.mustSyncFeed(mUserDid, list.getUri());
    case Role::ListHideReplies:
        return mUserSettings.getFeedHideReplies(mUserDid, list.getUri());
    case Role::ListHideFollowing:
        return mUserSettings.getFeedHideFollowing(mUserDid, list.getUri());
    case Role::MemberCountDelta:
        switch (memberCheck(list.getUri()))
        {
        case QEnums::TRIPLE_BOOL_UNKNOWN:
            return 0;
        case QEnums::TRIPLE_BOOL_NO:
            return (change && change->mMemberListItemUri && !change->mMemberListItemUri->isEmpty()) ? 1 : 0;
        case QEnums::TRIPLE_BOOL_YES:
            return (change && change->mMemberListItemUri && change->mMemberListItemUri->isEmpty()) ? -1 : 0;
        }

        return 0;
    case Role::MemberCheck:
        if (change && change->mMemberListItemUri)
            return change->mMemberListItemUri->isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;

        return memberCheck(list.getUri());
    case Role::MemberListItemUri:
        return change && change->mMemberListItemUri ? *change->mMemberListItemUri : getMemberListItemUri(list.getUri());
    case Role::EndOfList:
        return isEndOfList() && index.row() == (int)mList.size() - 1;
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void ListListModel::clear()
{
    qDebug() << "Clear lists";
    BaseType::clear();
    mListUriSet.clear();
    mMemberCheckResults.clear();
    clearLocalChanges();
    clearLocalProfileChanges();
}

int ListListModel::addLists(ATProto::AppBskyGraph::ListView::List lists, const QString& cursor)
{
    qDebug() << "Add lists:" << lists.size() << "cursor:" << cursor;
    Q_ASSERT(mMemberCheckDid.isEmpty());
    mCursor = cursor;

    auto filteredLists = filterLists(std::move(lists));

    if (filteredLists.empty())
    {
        qDebug() << "No new lists";
        setEndOfList();
        return 0;
    }

    const size_t newRowCount = mList.size() + filteredLists.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), filteredLists.begin(), filteredLists.end());
    endInsertRows();

    for (const auto& list : filteredLists)
        mListUriSet.insert(list.getUri());

    qDebug() << "New lists size:" << mList.size();
    return filteredLists.size();
}

int ListListModel::addLists(ATProto::AppBskyGraph::ListWithMembership::List listsWithMembership, const QString& cursor)
{
    qDebug() << "Add lists with membership:" << listsWithMembership.size() << "cursor:" << cursor;
    Q_ASSERT(!mMemberCheckDid.isEmpty());
    mCursor = cursor;
    ATProto::AppBskyGraph::ListView::List lists;
    lists.reserve(listsWithMembership.size());

    std::sort(listsWithMembership.begin(), listsWithMembership.end(),
            [](const auto& lhs, const auto& rhs){
                return SearchUtils::normalizedCompare(lhs->mList->mName, rhs->mList->mName) < 0;
            });

    for (const auto& list : listsWithMembership)
    {
        lists.push_back(list->mList);
        mMemberCheckResults[list->mList->mUri] = list->mListItem ? list->mListItem->mUri : "";
    }

    const auto filteredLists = filterLists(std::move(lists));

    if (filteredLists.empty())
    {
        qDebug() << "No new lists";
        setEndOfList();
        return 0;
    }

    const size_t newRowCount = mList.size() + filteredLists.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), filteredLists.begin(), filteredLists.end());
    endInsertRows();

    for (const auto& list : filteredLists)
        mListUriSet.insert(list.getUri());

    qDebug() << "New lists size:" << mList.size();
    return filteredLists.size();
}

void ListListModel::addLists(const QList<ListView>& lists)
{
    qDebug() << "Add lists:" << lists.size();
    Q_ASSERT(mMemberCheckDid.isEmpty());

    if (lists.empty())
    {
        qDebug() << "No new lists";
        return;
    }

    const size_t newRowCount = mList.size() + lists.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);
    mList.insert(mList.end(), lists.begin(), lists.end());
    endInsertRows();

    for (const auto& list : lists)
        mListUriSet.insert(list.getUri());

    qDebug() << "New lists size:" << mList.size();
}

void ListListModel::prependList(const ListView& list)
{
    qDebug() << "Prepend list:" << list.getName();

    beginInsertRows({}, 0, 0);

    mList.push_front(list);
    mListUriSet.insert(list.getUri());

    if (!mMemberCheckDid.isEmpty())
        mMemberCheckResults[list.getUri()] = "";

    endInsertRows();

    qDebug() << "New lists size:" << mList.size();
}

ListView ListListModel::updateEntry(int index, const QString& cid, const QString& name,
        const QString& description, const NamedLink::List& embeddedLinks, const QString& avatar)
{
    qDebug() << "Update entry:" << name << "index:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return {};
    }

    auto& list = mList[index];

    if (cid != list.getCid())
        list.setCid(cid);

    if (name != list.getName())
        list.setName(name);

    if (description != list.getDescription())
        list.setDescription(description, embeddedLinks);

    if (avatar != list.getAvatar())
        list.setAvatar(avatar);

    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    return list;
}

void ListListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return;
    }

    const QString listUri = mList[index].getUri();

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    endRemoveRows();

    mListUriSet.erase(listUri);

    if (!mMemberCheckDid.isEmpty())
        mMemberCheckResults.erase(listUri);
}

ListView ListListModel::getEntry(int index) const
{
    qDebug() << "Get entry:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return {};
    }

    return mList[index];
}

void ListListModel::setCursor(const QString& cursor)
{
    BaseType::setCursor(cursor);
    setEndOfList();
}

void ListListModel::setEndOfList()
{
    if (isEndOfList() && !mList.empty())
    {
        const auto index = createIndex(mList.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfList) });
    }
}

ListListModel::ListList ListListModel::filterLists(ATProto::AppBskyGraph::ListView::List lists) const
{
    ListList filtered;

    for (auto&& listView : lists)
    {
        if (mExcludeInternalLists && GraphUtils::isInternalList(*listView))
            continue;

        if (mPurpose == Purpose::LIST_PURPOSE_UNKNOWN &&
            listView->mPurpose == ATProto::AppBskyGraph::ListPurpose::REFERENCE_LIST)
        {
            // Filter out reference lists as those are used as internal lists by bsky, e.g.
            // for starter packs
            continue;
        }

        if (mPurpose == Purpose::LIST_PURPOSE_UNKNOWN ||
            listView->mPurpose == ATProto::AppBskyGraph::ListPurpose(mPurpose))
        {
            filtered.emplace_back(listView);
        }
    }

    return filtered;
}

QHash<int, QByteArray> ListListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::List), "list" },
        { int(Role::ListUri), "listUri" },
        { int(Role::ListName), "listName" },
        { int(Role::ListCreator), "listCreator" },
        { int(Role::ListBlockedUri), "listBlockedUri" },
        { int(Role::ListMuted), "listMuted" },
        { int(Role::ListSaved), "listSaved" },
        { int(Role::ListPinned), "listPinned" },
        { int(Role::ListHideFromTimeline), "listHideFromTimeline" },
        { int(Role::ListSync), "listSync" },
        { int(Role::ListHideReplies), "listHideReplies" },
        { int(Role::ListHideFollowing), "listHideFollowing" },
        { int(Role::MemberCountDelta), "memberCountDelta" },
        { int(Role::MemberCheck), "memberCheck" },
        { int(Role::MemberListItemUri), "memberListItemUri" },
        { int(Role::EndOfList), "endOfList" }
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
        const QString& listItemUri = it->second;
        return listItemUri.isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;
    }

    return QEnums::TRIPLE_BOOL_UNKNOWN;
}

QString ListListModel::getMemberListItemUri(const QString& listUri) const
{
    qDebug() << "Get member, list:" << listUri;

    if (mMemberCheckDid.isEmpty())
        return {};

    auto it = mMemberCheckResults.find(listUri);

    if (it == mMemberCheckResults.end())
        return {};

    qDebug() << "Get member, list:" << listUri << "item:" << it->second;
    return it->second;
}

void ListListModel::blockedChanged()
{
    changeData({ int(Role::ListBlockedUri) });
}

void ListListModel::mutedChanged()
{
    changeData({ int(Role::ListMuted) });
}

void ListListModel::hideFromTimelineChanged()
{
    changeData({ int(Role::ListHideFromTimeline) });
}

void ListListModel::syncListChanged()
{
    changeData({ int(Role::ListSync) });
}

void ListListModel::hideRepliesChanged()
{
    changeData({ int(Role::ListHideReplies) });
}

void ListListModel::hideFollowingChanged()
{
    changeData({ int(Role::ListHideFollowing) });
}

void ListListModel::memberListItemUriChanged()
{
    changeData({ int(Role::MemberCountDelta), int(Role::MemberCheck), int(Role::MemberListItemUri) });
}

void ListListModel::profileChanged()
{
    changeData({ int(Role::ListCreator) });
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
    emit dataChanged(createIndex(0, 0), createIndex(mList.size() - 1, 0), roles);
}

}
