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
    mDidConvoIdMap.clear();
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
        addConvoToDidMap(mConvos.back());
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
    addConvoToDidMap(mConvos[index]);
    changeData({ int(Role::Convo) }, index, index);
}

void ConvoListModel::updateBlockingUri(const QString& did, const QString& blockingUri)
{
    const auto& convoIds = mDidConvoIdMap[did];

    for (const auto& convoId : convoIds)
    {
        auto it = mConvoIdIndexMap.find(convoId);

        if (it == mConvoIdIndexMap.end())
            continue;

        const int index = it->second;

        if (!checkIndex(index))
            continue;

        mConvos[index].updateMemberBlocked(did, blockingUri);
        changeData({ int(Role::Convo) }, index, index);
    }
}

void ConvoListModel::insertConvo(const ConvoView& convo)
{
    qDebug() << "Insert convo:" << convo.getId() << "rev:" << convo.getRev();

    if (hasConvo(convo.getId()))
    {
        qWarning() << "Convo is already present";
        return;
    }

    int insertIndex = 0;
    for (; insertIndex < (int)mConvos.size(); ++insertIndex)
    {
        const auto& otherConvo = mConvos[insertIndex];

        if (convo.getRev() >= otherConvo.getRev())
        {
            qDebug() << "Insert index found:" << insertIndex << "convoRev:" << convo.getRev() << "otherRev:" << otherConvo.getRev();
            break;
        }
    }

    auto insertIt = insertIndex < (int)mConvos.size() ? mConvos.begin() + insertIndex : mConvos.end();

    beginInsertRows({}, insertIndex, insertIndex);
    mConvos.insert(insertIt, convo);
    addConvoToDidMap(convo);

    for (auto& [convoId, index] : mConvoIdIndexMap)
    {
        if (index >= insertIndex)
            ++index;
    }

    mConvoIdIndexMap[convo.getId()] = insertIndex;
    endInsertRows();

    setUnreadCount(mUnreadCount + convo.getUnreadCount());
}

bool ConvoListModel::checkIndex(int index) const
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < (int)mConvos.size());

    if (index < 0 || index >= (int)mConvos.size())
    {
        qWarning() << "Index out of range:" << index << "size:" << mConvos.size();
        return false;
    }

    return true;
}

void ConvoListModel::addConvoToDidMap(const ConvoView& convo)
{
    const QString& convoId = convo.getId();
    const auto& members = convo.getMembers();

    for (const auto& member : members)
    {
        const QString& did = member.getBasicProfile().getDid();
        mDidConvoIdMap[did].insert(convoId);
    }
}

void ConvoListModel::deleteConvo(const QString& convoId)
{
    qDebug() << "Delete convo:" << convoId;
    auto it = mConvoIdIndexMap.find(convoId);

    if (it == mConvoIdIndexMap.end())
    {
        qDebug() << "Convo not present:" << convoId;
        return;
    }

    const int index = it->second;

    if (!checkIndex(index))
        return;

    const int convoUnread = mConvos[index].getUnreadCount();
    setUnreadCount(mUnreadCount - convoUnread);

    beginRemoveRows({}, index, index);
    mConvos.erase(mConvos.begin() + index);
    mConvoIdIndexMap.erase(it);

    for (auto& [otherId, otherIndex] : mConvoIdIndexMap)
    {
        if (otherIndex > index)
            --otherIndex;
    }

    endRemoveRows();
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

    if (!checkIndex(index))
        return nullptr;

    return &mConvos[index];
}

QString ConvoListModel::getLastRevIncludingReactions() const
{
    QString rev = "";

    for (const auto& convo : mConvos)
    {
        if (convo.getRevIncludingReactions() > rev)
            rev = convo.getRevIncludingReactions();
    }

    return rev;
}

void ConvoListModel::setGetConvosInProgress(bool inProgress)
{
    if (inProgress != mGetConvosInProgress)
    {
        mGetConvosInProgress = inProgress;
        emit getConvosInProgressChanged();
    }
}

void ConvoListModel::setLoaded(bool loaded)
{
    if (loaded != mLoaded)
    {
        mLoaded = loaded;
        emit loadedChanged();
    }
}

void ConvoListModel::setUnreadCount(int unread)
{
    if (unread < 0)
    {
        qWarning() << "Negative unread:" << unread;
        unread = 0;
    }

    if (mUnreadCount != unread)
    {
        mUnreadCount = unread;
        emit unreadCountChanged();
    }
}

void ConvoListModel::updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output)
{
    int unread = mUnreadCount;

    for (const auto& convo : output.mConvos)
    {
        if (!convo->mMuted)
            unread += convo->mUnreadCount;
    }

    setUnreadCount(unread);
}

BasicProfileList ConvoListModel::getAllConvoMembers() const
{
    std::unordered_set<QString> usedDids;
    usedDids.insert(mUserDid);
    BasicProfileList allConvoMembers;

    for (const auto& convo : mConvos)
    {
        const ChatBasicProfileList& members = convo.getMembers();

        for (const auto& member : members)
        {
            const auto& profile = member.getBasicProfile();

            if (!usedDids.contains(profile.getDid()))
            {
                allConvoMembers.push_back(profile);
                usedDids.insert(profile.getDid());
            }
        }
    }

    return allConvoMembers;
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
