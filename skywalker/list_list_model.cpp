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
    QAbstractListModel(parent),
    mType(type),
    mPurpose(purpose),
    mAtId(atId),
    mFavoriteFeeds(favoriteFeeds),
    mUserDid(skywalker->getUserDid()),
    mTimelineHide(*skywalker->getTimelineHide()),
    mUserSettings(*skywalker->getUserSettings())
{
    qDebug() << "New list list model type:" << type << "purpose:" << purpose << "atId:" << atId;

    connect(&mFavoriteFeeds, &FavoriteFeeds::listSaved, this, [this]{ listSavedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listPinned, this, [this]{ listPinnedChanged(); });
    connect(&mFavoriteFeeds, &FavoriteFeeds::listUnpinned, this, [this](QString){ listPinnedChanged(); });
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
        return 0;
    }

    std::sort(filteredLists.begin(), filteredLists.end(),
            [](const auto& lhs, const auto& rhs){
                return SearchUtils::normalizedCompare(lhs.getName(), rhs.getName()) < 0;
            });

    const size_t newRowCount = mLists.size() + filteredLists.size();

    beginInsertRows({}, mLists.size(), newRowCount - 1);
    mLists.insert(mLists.end(), filteredLists.begin(), filteredLists.end());
    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
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
    Q_ASSERT(mMemberCheckDid.isEmpty());

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

    if (!mMemberCheckDid.isEmpty())
        mMemberCheckResults[list.getUri()] = "";

    endInsertRows();

    qDebug() << "New lists size:" << mLists.size();
}

ListView ListListModel::updateEntry(int index, const QString& cid, const QString& name,
        const QString& description, const WebLink::List& embeddedLinks, const QString& avatar)
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
        list.setDescription(description, embeddedLinks);

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

ListListModel::ListList ListListModel::filterLists(ATProto::AppBskyGraph::ListView::List lists) const
{
    ListList filtered;

    for (auto&& listView : lists)
    {
        if (mExcludeInternalLists && GraphUtils::isInternalList(listView->mUri))
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

void ListListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
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
    changeData({ int(Role::MemberCheck), int(Role::MemberListItemUri) });
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
    emit dataChanged(createIndex(0, 0), createIndex(mLists.size() - 1, 0), roles);
}

}
