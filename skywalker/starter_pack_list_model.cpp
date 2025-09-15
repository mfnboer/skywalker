// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "starter_pack_list_model.h"

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

    switch (Role(role))
    {
    case Role::StarterPack:
        return QVariant::fromValue(starterPack);
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

void StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackViewBasicList starterPacks, const QString& cursor)
{
    _addStarterPacks(starterPacks, cursor);
}

void StarterPackListModel::addStarterPacks(ATProto::AppBskyGraph::StarterPackView::List starterPacks, const QString& cursor)
{
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

    const size_t newRowCount = mStarterPacks.size() + starterPacks.size();

    beginInsertRows({}, mStarterPacks.size(), newRowCount - 1);

    for (auto& starterPack : starterPacks)
        mStarterPacks.emplace_back(starterPack);

    endInsertRows();
    qDebug() << "New starter packs size:" << mStarterPacks.size();
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
        { int(Role::StarterPack), "starterPack" }
    };

    return roles;
}

}
