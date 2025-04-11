// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "message_view.h"
#include <QAbstractListModel>
#include <deque>

namespace Skywalker
{

class MessageListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Message = Qt::UserRole + 1,
        SenderIsUser,
        SameSenderAsNext,
        SameSenderAsPrevious,
        SameTimeAsNext,
        SameDateAsPrevious,
        EndOfList
    };

    using Ptr = std::unique_ptr<MessageListModel>;

    explicit MessageListModel(const QString& userDid, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void clear();
    void addMessages(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageList& messages, const QString& cursor);
    void updateMessages(const ATProto::ChatBskyConvo::GetMessagesOutput::MessageList& messages, const QString& cursor);
    void updateMessage(const MessageView& msg);
    const QString& getCursor() const { return mCursor; }
    bool isEndOfList() const { return mCursor.isEmpty(); }
    const MessageView* getLastMessage() const;

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    int getMessageIndexById(const QString& id) const;
    void rebuildIndex();
    void changeData(const QList<int>& roles, int begin = 0, int end = -1);

    const QString& mUserDid;

    // Ordered from oldest to newest
    std::deque<MessageView> mMessages;
    std::unordered_map<QString, int> mMessageIdToPosIndex; // to position in mMessages
    QString mCursor;
};

}
