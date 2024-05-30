// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "convo_list_model.h"

namespace Skywalker {

ConvoListModel::ConvoListModel(const QString& userDid, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid)
{
}

int ConvoListModel::rowCount(const QModelIndex&) const
{
    return mConvos.size();
}

QVariant ConvoListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= (int)mConvos.size())
        return {};

    const auto& convo = mConvos[index.row()];

    switch (Role(role))
    {
    case Role::Convo:
        return QVariant::fromValue(convo);
    case Role::EndOfList:
        return index.row() == (int)mConvos.size() - 1 && isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void ConvoListModel::clear()
{
    qDebug() << "Clear convos";

    if (!mConvos.empty())
    {
        beginRemoveRows({}, 0, mConvos.size() - 1);
        mConvos.clear();
        endRemoveRows();
    }

    mConvoIdIndexMap.clear();
    mCursor.clear();
}

void ConvoListModel::addConvos(const ATProto::ChatBskyConvo::ConvoViewList& convos, const QString& cursor)
{
    qDebug() << "Add convos:" << convos.size() << "cursor:" << cursor;
    mCursor = cursor;
    changeData({ int(Role::EndOfList) }, (int)mConvos.size() - 1, (int)mConvos.size() - 1);

    if (convos.empty())
    {
        qDebug() << "No new convos";
        return;
    }

    const size_t newRowCount = mConvos.size() + convos.size();
    beginInsertRows({}, mConvos.size(), newRowCount - 1);
    mConvos.reserve(newRowCount);

    for (const auto& convo : convos)
    {
        qDebug() << "New convo, id:" << convo->mId << "rev:" << convo->mRev;
        mConvos.emplace_back(*convo, mUserDid);
        mConvoIdIndexMap[convo->mId] = mConvos.size() - 1;
    }

    endInsertRows();
    qDebug() << "New convos size:" << mConvos.size();
}

void ConvoListModel::updateConvo(const ATProto::ChatBskyConvo::ConvoView& convo)
{
    const auto* oldConvo = getConvo(convo.mId);

    if (!oldConvo)
        return;

    Q_ASSERT(oldConvo->getId() == convo.mId);

    if (oldConvo->getId() != convo.mId)
    {
        qWarning() << "Non-matching convo:" << oldConvo->getId() << convo.mId;
        return;
    }

    auto it = mConvoIdIndexMap.find(convo.mId);
    const int index = it->second;
    mConvos[index] = ConvoView{convo, mUserDid};
    changeData({ int(Role::Convo) }, index, index);
}

const ConvoView* ConvoListModel::getConvo(const QString& convoId) const
{
    auto it = mConvoIdIndexMap.find(convoId);

    if (it == mConvoIdIndexMap.end())
    {
        qWarning() << "Cannot find convo:" << convoId;
        return nullptr;
    }

    const int index = it->second;
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < (int)mConvos.size());

    if (index < 0 || index >= (int)mConvos.size())
    {
        qWarning() << "Index out of range:" << index << "size:" << mConvos.size();
        return nullptr;
    }

    return &mConvos[index];
}

QString ConvoListModel::getLastRev() const
{
    if (mConvos.empty())
        return "";

    return mConvos.front().getRev();
}

QHash<int, QByteArray> ConvoListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Convo), "convo" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void ConvoListModel::changeData(const QList<int>& roles, int begin, int end)
{
    if (end < 0)
        end = (int)mConvos.size() - 1;

    if (begin < 0 || begin >= (int)mConvos.size() || end < 0 || end >= (int)mConvos.size())
        return;

    emit dataChanged(createIndex(begin, 0), createIndex(end, 0), roles);
}

}
