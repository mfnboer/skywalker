// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "message_list_model.h"

namespace Skywalker {

MessageListModel::MessageListModel(const QString& userDid, QObject* parent) :
    QAbstractListModel(parent),
    mUserDid(userDid)
{
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

        const auto msgTime = message.getSentAt().time();
        const auto nextTime = nextMessage->getSentAt().time();
        return msgTime.hour() == nextTime.hour() && msgTime.minute() == nextTime.minute();
    }
    case Role::EndOfList:
        return index.row() == (int)mMessages.size() - 1 && isEndOfList();
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
        endRemoveRows();
    }

    mCursor.clear();
}

void MessageListModel::addMessages(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageList& messages, const QString& cursor)
{
    qDebug() << "Add messages:" << messages.size() << "cursor:" << cursor;
    mCursor = cursor;
    changeData({ int(Role::EndOfList) }, (int)mMessages.size() - 1, (int)mMessages.size() - 1);

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

        const auto* messageView = std::get_if<ATProto::ChatBskyConvo::MessageView::Ptr>(&message);
        const auto* deletedMessageView = std::get_if<ATProto::ChatBskyConvo::DeletedMessageView::Ptr>(&message);

        if (messageView)
        {
            const auto& msg = *messageView;
            qDebug() << "New message, id:" << msg->mId << "rev:" << msg->mRev << "text:" << msg->mText;
            mMessages.emplace_front(*msg);
        }
        else if (deletedMessageView)
        {
            const auto& msg = *deletedMessageView;
            qDebug() << "Deleted message, ud:" << msg->mId << "rev:" << msg->mRev;
            mMessages.emplace_front(*msg);
        }
    }

    endInsertRows();
    qDebug() << "New messages size:" << mMessages.size();
}

QHash<int, QByteArray> MessageListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        { int(Role::Message), "message" },
        { int(Role::SenderIsUser), "senderIsUser" },
        { int(Role::SameSenderAsNext), "sameSenderAsNext" },
        { int(Role::SameSenderAsPrevious), "sameSenderAsPrevious" },
        { int(Role::SameTimeAsNext), "sameTimeAsNext" },
        { int(Role::EndOfList), "endOfList" }
    };

    return roles;
}

void MessageListModel::changeData(const QList<int>& roles, int begin, int end)
{
    if (end < 0)
        end = (int)mMessages.size() - 1;

    if (begin < 0 || begin >= (int)mMessages.size() || end < 0 || end >= (int)mMessages.size())
        return;

    emit dataChanged(createIndex(begin, 0), createIndex(end, 0), roles);
}

}
