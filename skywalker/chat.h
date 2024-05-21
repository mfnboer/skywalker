// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_list_model.h"
#include "message_list_model.h"
#include <atproto/lib/client.h>

namespace Skywalker {

class Chat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConvoListModel* convoListModel READ getConvoListModel CONSTANT FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount NOTIFY unreadCountChanged FINAL)
    Q_PROPERTY(bool getConvosInProgress READ isGetConvosInProgress NOTIFY getConvosInProgressChanged FINAL)
    Q_PROPERTY(bool getMessagesInProgress READ isGetMessagesInProgress NOTIFY getMessagesInProgressChanged FINAL)

public:
    explicit Chat(ATProto::Client::Ptr& bsky, const QString& mUserDid, QObject* parent = nullptr);

    void clear();
    Q_INVOKABLE void getConvos(const QString& cursor = "");
    Q_INVOKABLE void getConvosNextPage();
    Q_INVOKABLE bool convosLoaded() const { return mLoaded; }

    ConvoListModel* getConvoListModel() { return &mConvoListModel; }
    int getUnreadCount() const { return mUnreadCount; }

    bool isGetConvosInProgress() const { return mGetConvosInProgress; }
    void setConvosInProgress(bool inProgress);

    Q_INVOKABLE MessageListModel* getMessageListModel(const QString& convoId);
    Q_INVOKABLE void getMessages(const QString& convoId, const QString& cursor = "");
    Q_INVOKABLE void getMessagesNextPage(const QString& convoId);
    Q_INVOKABLE void updateRead(const QString& convoId, int readCount);

    bool isGetMessagesInProgress() const { return mGetMessagesInProgress; }
    void setMessagesInProgress(bool inProgress);

signals:
    void unreadCountChanged();
    void getConvosInProgressChanged();
    void getConvosFailed(QString error);
    void getMessagesInProgressChanged();
    void getMessagesFailed(QString error);
    // TODO: handle failures

private:
    void setUnreadCount(int unread);
    void updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output);

    ATProto::Client::Ptr& mBsky;
    const QString& mUserDid;
    ConvoListModel mConvoListModel;
    int mUnreadCount = 0;
    bool mGetConvosInProgress = false;
    bool mLoaded = false;
    std::unordered_map<QString, MessageListModel::Ptr> mMessageListModels; // convoId -> model
    bool mGetMessagesInProgress = false;
};

}
