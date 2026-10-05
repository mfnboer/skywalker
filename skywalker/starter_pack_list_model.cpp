// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "starter_pack_list_model.h"
#include "search_utils.h"

namespace Skywalker {

StarterPackListModel::StarterPackListModel(QObject* parent) :
    QAbstractListModel(parent)
{
}

int StarterPackListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mStarterPacks.size();
}

QVariant StarterPackListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mStarterPacks.size())
        return {};

    const auto& starterPack = mStarterPacks[index.row()];
    const auto* listChange = !starterPack.isNull() ? LocalListModelChanges::getLocalChange(starterPack.getList().getUri()) : nullptr;

    switch (Role(role))
    {
    case Role::StarterPack:
        return QVariant::fromValue(starterPack);
    case Role::MemberCountDelta:
        switch (memberCheck(starterPack.getUri()))
        {
        case QEnums::TRIPLE_BOOL_UNKNOWN:
            return 0;
        case QEnums::TRIPLE_BOOL_NO:
            return (listChange && listChange->mMemberListItemUri && !listChange->mMemberListItemUri->isEmpty()) ? 1 : 0;
        case QEnums::TRIPLE_BOOL_YES:
            return (listChange && listChange->mMemberListItemUri && listChange->mMemberListItemUri->isEmpty()) ? -1 : 0;
        }

        return 0;
    case Role::MemberCheck:
        if (listChange && listChange->mMemberListItemUri)
            return listChange->mMemberListItemUri->isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;

        return memberCheck(starterPack.getUri());
    case Role::MemberListItemUri:
        return listChange && listChange->mMemberListItemUri ? *listChange->mMemberListItemUri : getMemberListItemUri(starterPack.getUri());
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void StarterPackListModel::clear()
{
    qDebug() << "Clear starter packs";

    if (!mStarterPacks.empty())
    {
        beginRemoveRows({}, 0, mStarterPacks.size() - 1);
        mStarterPacks.clear();
        endRemoveRows();
    }

    mCursor.clear();
}

void StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasic::List starterPacks, const QString& cursor)
{
    _addStarterPacks(starterPacks, cursor);
}

void StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackView::List starterPacks, const QString& cursor)
{
    _addStarterPacks(starterPacks, cursor);
}

void StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackWithMembership::List starterPacksWithMembership, const QString& cursor)
{
    qDebug() << "Add starter pack with membership:" << starterPacksWithMembership.size() << "cursor:" << cursor;
    Q_ASSERT(!mMemberCheckDid.isEmpty());
    mCursor = cursor;
    ATProto::AppBskyGraph::StarterPackView::List starterPacks;
    starterPacks.reserve(starterPacksWithMembership.size());

    for (const auto& starterPack : starterPacksWithMembership)
    {
        qDebug() << "NAME:" << starterPack->mStarterPack->getName() << "ITEM:" << (starterPack->mListItem ? starterPack->mListItem->mUri : "");
        starterPacks.push_back(starterPack->mStarterPack);
        mMemberCheckResults[starterPack->mStarterPack->mUri] = (starterPack->mListItem ? starterPack->mListItem->mUri : "");
    }

    _addStarterPacks(starterPacks, cursor);
}

template<typename T>
void StarterPackListModel::_addStarterPacks(const std::vector<T>& starterPacks, const QString& cursor)
{
    qDebug() << "Add starter packs:" << starterPacks.size() << "cursor:" << cursor;
    mCursor = cursor;

    if (starterPacks.empty())
    {
        qDebug() << "No new starter packs";
        return;
    }

    auto sortedStarterPacks = starterPacks;

    std::sort(sortedStarterPacks.begin(), sortedStarterPacks.end(),
              [](const auto& lhs, const auto& rhs){
                  return SearchUtils::normalizedCompare(lhs->getName(), rhs->getName()) < 0;
              });

    const size_t newRowCount = mStarterPacks.size() + sortedStarterPacks.size();

    beginInsertRows({}, mStarterPacks.size(), newRowCount - 1);

    for (auto& starterPack : sortedStarterPacks)
        mStarterPacks.emplace_back(starterPack);

    endInsertRows();
    qDebug() << "New starter packs size:" << mStarterPacks.size();
}

void StarterPackListModel::prependStarterPack(const StarterPackView& starterPack)
{
    qDebug() << "Prepend starter pack:" << starterPack.getName();

    beginInsertRows({}, 0, 0);
    mStarterPacks.push_front(starterPack);
    endInsertRows();

    qDebug() << "New starter packs size:" << mStarterPacks.size();
}

StarterPackViewBasic StarterPackListModel::updateEntry(
    int index, const QString& cid, const QString& name,
    const QString& description, const NamedLink::List& embeddedLinks)
{
    qDebug() << "Update entry:" << name << "index:" << index;

    if (index < 0 || (size_t)index >= mStarterPacks.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mStarterPacks.size();
        return {};
    }

    auto& starterPack = mStarterPacks[index];

    if (cid != starterPack.getCid())
        starterPack.setCid(cid);

    if (name != starterPack.getName())
        starterPack.setName(name);

    if (description != starterPack.getDescription())
        starterPack.setDescription(description, embeddedLinks);

    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    return starterPack;
}

void StarterPackListModel::deleteEntry(int index)
{
    qDebug() << "Delete entry:" << index;

    if (index < 0 || (size_t)index >= mStarterPacks.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mStarterPacks.size();
        return;
    }

    beginRemoveRows({}, index, index);
    mStarterPacks.erase(mStarterPacks.begin() + index);
    endRemoveRows();
}

StarterPackViewBasic StarterPackListModel::getEntry(int index) const
{
    qDebug() << "Get entry:" << index;

    if (index < 0 || (size_t)index >= mStarterPacks.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mStarterPacks.size();
        return {};
    }

    return mStarterPacks[index];
}

void StarterPackListModel::setGetFeedInProgress(bool inProgress)
{
    if (inProgress != mGetFeedInProgress) {
        mGetFeedInProgress = inProgress;
        emit getFeedInProgressChanged();
    }
}

QHash<int, QByteArray> StarterPackListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::StarterPack), "starterPack" },
        { int(Role::MemberCountDelta), "memberCountDelta" },
        { int(Role::MemberCheck), "memberCheck" },
        { int(Role::MemberListItemUri), "memberListItemUri" }
    };

    return roles;
}

QEnums::TripleBool StarterPackListModel::memberCheck(const QString& starterPackUri) const
{
    if (mMemberCheckDid.isEmpty())
        return QEnums::TRIPLE_BOOL_UNKNOWN;

    auto it = mMemberCheckResults.find(starterPackUri);

    if (it != mMemberCheckResults.end())
    {
        const QString& listItemUri = it->second;
        return listItemUri.isEmpty() ? QEnums::TRIPLE_BOOL_NO : QEnums::TRIPLE_BOOL_YES;
    }

    return QEnums::TRIPLE_BOOL_UNKNOWN;
}

QString StarterPackListModel::getMemberListItemUri(const QString& starterPackUri) const
{
    qDebug() << "Get member, starter pack:" << starterPackUri;

    if (mMemberCheckDid.isEmpty())
        return {};

    auto it = mMemberCheckResults.find(starterPackUri);

    if (it == mMemberCheckResults.end())
        return {};

    qDebug() << "Get member, starter pack:" << starterPackUri << "item:" << it->second;
    return it->second;
}

void StarterPackListModel::memberListItemUriChanged()
{
    changeData({ int(Role::MemberCountDelta), int(Role::MemberCheck), int(Role::MemberListItemUri) });
}

void StarterPackListModel::changeData(const QList<int>& roles)
{
    emit dataChanged(createIndex(0, 0), createIndex(mStarterPacks.size() - 1, 0), roles);
}

}
