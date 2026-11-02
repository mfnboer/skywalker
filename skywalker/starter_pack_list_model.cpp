// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "starter_pack_list_model.h"
#include "search_utils.h"

namespace Skywalker {

StarterPackListModel::StarterPackListModel(Type type, const QString& atId, const QString& userDid, QObject* parent) :
    BaseType(userDid, parent),
    mType(type),
    mAtId(atId)
{
}

QVariant StarterPackListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mList.size())
        return {};

    const auto& starterPack = mList[index.row()];
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
    case Role::OptOutUri:
        return listChange && listChange->mOptOutUri ? *listChange->mOptOutUri : starterPack.getList().getViewer().getReferenceListOptOut();
    case Role::EndOfFeed:
        return index.row() == (int)mList.size() - 1 && isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void StarterPackListModel::clear()
{
    qDebug() << "Clear starter packs";
    BaseType::clear();
    mStarterPackUriSet.clear();
}

int StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasic::List starterPacks, const QString& cursor)
{
    return _addStarterPacks(starterPacks, cursor);
}

int StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackView::List starterPacks, const QString& cursor)
{
    return _addStarterPacks(starterPacks, cursor);
}

int StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackWithMembership::List starterPacksWithMembership, const QString& cursor)
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

    return _addStarterPacks(starterPacks, cursor, true);
}

template<typename T>
int StarterPackListModel::_addStarterPacks(const std::vector<T>& starterPacks, const QString& cursor, bool sort)
{
    qDebug() << "Add starter packs:" << starterPacks.size() << "cursor:" << cursor;
    mCursor = cursor;

    if (starterPacks.empty())
    {
        qDebug() << "No new starter packs";

        if (!mList.empty())
            emit dataChanged(createIndex(mList.size() - 1, 0), createIndex(mList.size() - 1, 0), { (int)Role::EndOfFeed });

        return 0;
    }

    auto sortedStarterPacks = starterPacks;

    if (sort)
    {
        std::sort(sortedStarterPacks.begin(), sortedStarterPacks.end(),
                  [](const auto& lhs, const auto& rhs){
                      return SearchUtils::normalizedCompare(lhs->getName(), rhs->getName()) < 0;
                  });
    }

    const size_t newRowCount = mList.size() + sortedStarterPacks.size();

    beginInsertRows({}, mList.size(), newRowCount - 1);

    for (auto& starterPack : sortedStarterPacks)
    {
        mList.emplace_back(starterPack);
        mStarterPackUriSet.insert(starterPack->mUri);
    }

    endInsertRows();
    qDebug() << "New starter packs size:" << mList.size();
    return sortedStarterPacks.size();
}

void StarterPackListModel::prependStarterPack(const StarterPackView& starterPack)
{
    qDebug() << "Prepend starter pack:" << starterPack.getName();

    beginInsertRows({}, 0, 0);
    mList.push_front(starterPack);
    mStarterPackUriSet.insert(starterPack.getUri());
    endInsertRows();

    qDebug() << "New starter packs size:" << mList.size();
}

StarterPackViewBasic StarterPackListModel::updateEntry(
    int index, const QString& cid, const QString& name,
    const QString& description, const NamedLink::List& embeddedLinks)
{
    qDebug() << "Update entry:" << name << "index:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return {};
    }

    auto& starterPack = mList[index];

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

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return;
    }

    const QString starterPackUri = mList[index].getUri();

    beginRemoveRows({}, index, index);
    mList.erase(mList.begin() + index);
    mStarterPackUriSet.erase(starterPackUri);
    endRemoveRows();
}

StarterPackViewBasic StarterPackListModel::getEntry(int index) const
{
    qDebug() << "Get entry:" << index;

    if (index < 0 || (size_t)index >= mList.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mList.size();
        return {};
    }

    return mList[index];
}

void StarterPackListModel::setCursor(const QString& cursor)
{
    BaseType::setCursor(cursor);
    setEndOfList();
}

void StarterPackListModel::setEndOfList()
{
    if (isEndOfList() && !mList.empty())
    {
        const auto index = createIndex(mList.size() - 1, 0);
        emit dataChanged(index, index, { int(Role::EndOfFeed) });
    }
}

QHash<int, QByteArray> StarterPackListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::StarterPack), "starterPack" },
        { int(Role::MemberCountDelta), "memberCountDelta" },
        { int(Role::MemberCheck), "memberCheck" },
        { int(Role::MemberListItemUri), "memberListItemUri" },
        { int(Role::OptOutUri), "optOutUri" },
        { int(Role::EndOfFeed), "endOfFeed" }
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

void StarterPackListModel::optOutUriChanged()
{
    changeData({ int(Role::OptOutUri) });
}

}
