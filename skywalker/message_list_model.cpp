// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_list_model.h"

namespace Skywalker {

using namespace std::chrono_literals;

MessageListModel::MessageListModel(const QString& userDid, const ChatBasicProfileList& members, FollowsActivityStore& followsActivityStore, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid),
    mFollowsActivityStore(followsActivityStore)
{
    for (const auto& member : members)
        mDidMemberMap[member.getBasicProfile().getDid()] = member;
}

int MessageListModel::rowCount(const QModelIndex&) const
{
    return mMessages.size();
}

QVariant MessageListModel::data(const QModelIndex& index, int role) const
{
    static const MessageView NULL_MESSAGE;

    if (index.row() < 0 || index.row() >= (int)mMessages.size())
        return {};

    const auto& message = mMessages[index.row()];
    const auto* nextMessage = index.row() < (int)mMessages.size() - 1 ? &mMessages[index.row() + 1] : &NULL_MESSAGE;
    const auto* prevMessage = index.row() > 0 ? &mMessages[index.row() - 1] : &NULL_MESSAGE;

    switch (Role(role))
    {
    case Role::Message:
        return QVariant::fromValue(message);
    case Role::SenderIsUser:
        return message.getSenderDid() == mUserDid;
    case Role::SameSenderAsNext:
        return !nextMessage->isNull() && message.getSenderDid() == nextMessage->getSenderDid();
    case Role::SameSenderAsPrevious:
        return !prevMessage->isNull() && message.getSenderDid() == prevMessage->getSenderDid();
    case Role::SameTimeAsNext:
    {
        if (nextMessage->isNull())
            return false;

        return nextMessage->getSentAt() - message.getSentAt() < 5min;
    }
    case Role::SameDateAsPrevious:
        return !prevMessage->isNull() && message.getSentAt().toLocalTime().date() == prevMessage->getSentAt().toLocalTime().date();
    case Role::EndOfList:
        return index.row() == 0 && isEndOfList();
    }

    qWarning() << "Uknown role requested:" << role;
    return {};
}

void MessageListModel::clear()
{
    qDebug() << "Clear messages";

    if (!mMessages.empty())
    {
        beginRemoveRows({}, 0, mMessages.size() - 1);
        mMessages.clear();
        mMessageIdToPosIndex.clear();
        endRemoveRows();
    }

    mCursor.clear();
}

void MessageListModel::addMessages(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageList& messages, const QString& cursor)
{
    qDebug() << "Add messages:" << messages.size() << "cursor:" << cursor;
    mCursor = cursor;
    changeData({ int(Role::EndOfList) }, 0, 0);

    if (messages.empty())
    {
        qDebug() << "No new messages";
        return;
    }

    beginInsertRows({}, 0, messages.size() - 1);

    for (const auto& message : messages)
    {
        if (ATProto::isNullVariant(message))
        {
            qWarning() << "Unknown message";
            continue;
        }

        mMessages.emplace_front(message);
        reportActivity(mMessages.front());
    }

    rebuildIndex();

    endInsertRows();
    qDebug() << "New messages size:" << mMessages.size();
}

void MessageListModel::updateMessages(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageList& messages, const QString& cursor)
{
    if (mMessages.empty())
    {
        clear();
        addMessages(messages, cursor);
        return;
    }

    if (messages.empty())
    {
        clear();
        addMessages(messages, cursor);
        return;
    }

    const MessageView& firstStoredMsg = mMessages.back();
    const MessageView firstNewMsg(messages.front());
    qDebug() << "First stored msg id:" << firstStoredMsg.getId() << "new msg id:" << firstNewMsg.getId();

    if (firstStoredMsg.getId() != firstNewMsg.getId())
    {
        clear();
        addMessages(messages, cursor);
        return;
    }

    // No new messages, but existing messages could be updated, e.g. reactions
    for (const auto& msg : messages)
    {
        const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::SharedPtr>(&msg);

        if (!messageView)
            continue;

        const QString& msgId = (*messageView)->mId;
        const int index = getMessageIndexById(msgId);

        if (index < 0)
            continue;

        MessageView& storedMsg = mMessages[index];

        if (storedMsg.isDeleted())
            continue;

        const QString& msgRev = (*messageView)->mRev;

        if (msgRev <= storedMsg.getRev())
            continue;

        qDebug() << "Update existing message:" << storedMsg.getId() << "oldRev:" << storedMsg.getRev() << "newRev:" << msgRev;
        const MessageView updatedMsg(**messageView);
        storedMsg = updatedMsg;
        reportActivity(storedMsg);
        changeData({}, index, index);
    }
}

void MessageListModel::updateMessage(const MessageView& msg)
{
    qDebug() << "Update message:" << msg.getId() << "rev:" << msg.getRev();
    const int index = getMessageIndexById(msg.getId());

    if (index < 0) {
        qWarning() << "Message not found:" << msg.getId();
        return;
    }

    MessageView& storedMsg = mMessages[index];
    qDebug() << "Found message:" << storedMsg.getId() << "rev:" << storedMsg.getRev();
    storedMsg = msg;
    reportActivity(storedMsg);
    changeData({}, index, index);
}

const MessageView* MessageListModel::getLastMessage() const
{
    if (mMessages.empty())
        return nullptr;

    return &mMessages.back();
}

QHash<int, QByteArray> MessageListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Message), "message" },
        { int(Role::SenderIsUser), "senderIsUser" },
        { int(Role::SameSenderAsNext), "sameSenderAsNext" },
        { int(Role::SameSenderAsPrevious), "sameSenderAsPrevious" },
        { int(Role::SameTimeAsNext), "sameTimeAsNext" },
        { int(Role::SameDateAsPrevious), "sameDateAsPrevious" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

int MessageListModel::getMessageIndexById(const QString& id) const
{
    auto it = mMessageIdToPosIndex.find(id);

    if (it == mMessageIdToPosIndex.end())
        return -1;

    const int index = it->second;

    if (index < 0 || index >= (int)mMessages.size())
    {
        qWarning() << "Invalid index:" << index << "size:" << mMessages.size();
        return -1;
    }

    return index;
}

void MessageListModel::rebuildIndex()
{
    mMessageIdToPosIndex.clear();

    for (int i = 0; i < (int)mMessages.size(); ++i)
    {
        const auto& msg = mMessages[i];
        mMessageIdToPosIndex[msg.getId()] = i;
    }
}

void MessageListModel::changeData(const QList<int>& roles, int begin, int end)
{
    if (end < 0)
        end = (int)mMessages.size() - 1;

    if (begin < 0 || begin >= (int)mMessages.size() || end < 0 || end >= (int)mMessages.size())
        return;

    emit dataChanged(createIndex(begin, 0), createIndex(end, 0), roles);
}

void MessageListModel::reportActivity(const MessageView& message)
{
    if (message.isNull())
        return;

    const auto& did = message.getSenderDid();
    const auto timestamp = message.getSentAt();

    if (!did.isEmpty() && timestamp.isValid())
    {
        const auto it = mDidMemberMap.find(did);

        if (it != mDidMemberMap.end())
            mFollowsActivityStore.reportActivity(it->second.getBasicProfile(), timestamp);
    }

    for (const auto& reaction : message.getReactions())
        reportActivity(reaction);
}

void MessageListModel::reportActivity(const ReactionView& reaction)
{
    if (reaction.isNull())
        return;

    const auto& did = reaction.getSenderDid();
    const auto timestamp = reaction.getCreatedAt();

    if (!did.isEmpty() && timestamp.isValid())
    {
        const auto it = mDidMemberMap.find(did);

        if (it != mDidMemberMap.end())
            mFollowsActivityStore.reportActivity(it->second.getBasicProfile(), timestamp);
    }
}

}
