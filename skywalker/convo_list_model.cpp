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

    mCursor.clear();
}

void ConvoListModel::addConvos(const ATProto::ChatBskyConvo::ConvoViewList& convos, const QString& cursor)
{
    qDebug() << "Add convos:" << convos.size() << "cursor:" << cursor;
    mCursor = cursor;

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
    }

    endInsertRows();
    qDebug() << "New convos size:" << mConvos.size();
}

QHash<int, QByteArray> ConvoListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Convo), "convo" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

}
