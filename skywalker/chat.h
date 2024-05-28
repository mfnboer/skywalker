// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_list_model.h"
#include "message_list_model.h"
#include "presence.h"
#include <atproto/lib/chat_master.h>
#include <atproto/lib/client.h>
#include <atproto/lib/post_master.h>

namespace Skywalker {

class Chat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConvoListModel* convoListModel READ getConvoListModel CONSTANT FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount NOTIFY unreadCountChanged FINAL)
    Q_PROPERTY(bool getConvosInProgress READ isGetConvosInProgress NOTIFY getConvosInProgressChanged FINAL)
    Q_PROPERTY(bool startConvoInProgress READ isStartConvoInProgress NOTIFY startConvoInProgressChanged FINAL)
    Q_PROPERTY(bool getMessagesInProgress READ isGetMessagesInProgress NOTIFY getMessagesInProgressChanged FINAL)

public:
    explicit Chat(ATProto::Client::Ptr& bsky, const QString& mUserDid, QObject* parent = nullptr);

    void reset();
    void initSettings();
    void updateSettings(QEnums::AllowIncomingChat allowIncoming);
    QEnums::AllowIncomingChat getAllowIncomingChat() const { return mAllowIncomingChat; }
    QString getLastRev() const;

    Q_INVOKABLE void getConvos(const QString& cursor = "");
    Q_INVOKABLE void getConvosNextPage();
    Q_INVOKABLE void updateConvos();
    Q_INVOKABLE void startConvoForMembers(const QStringList& dids);
    Q_INVOKABLE void startConvoForMember(const QString& did);
    Q_INVOKABLE void leaveConvo(const QString& convoId);
    Q_INVOKABLE void muteConvo(const QString& convoId);
    Q_INVOKABLE void unmuteConvo(const QString& convoId);
    Q_INVOKABLE bool convosLoaded() const { return mLoaded; }

    ConvoListModel* getConvoListModel() { return &mConvoListModel; }
    int getUnreadCount() const { return mUnreadCount; }

    bool isGetConvosInProgress() const { return mGetConvosInProgress; }
    void setConvosInProgress(bool inProgress);

    bool isStartConvoInProgress() const { return mStartConvoInProgress; }
    void setStartConvoInProgress(bool inProgress);

    Q_INVOKABLE MessageListModel* getMessageListModel(const QString& convoId);
    Q_INVOKABLE void removeMessageListModel(const QString& convoId);
    Q_INVOKABLE bool messageConvoOpen() const { return !mMessageListModels.empty(); }

    Q_INVOKABLE void getMessages(const QString& convoId, const QString& cursor = "");
    Q_INVOKABLE void getMessagesNextPage(const QString& convoId);
    Q_INVOKABLE void updateMessages(const QString& convoId);
    Q_INVOKABLE void updateRead(const QString& convoId);

    Q_INVOKABLE void sendMessage(const QString& convoId, const QString& text, const QString& quoteUri, const QString& quoteCid);
    Q_INVOKABLE void deleteMessage(const QString& convoId, const QString& messageId);

    bool isGetMessagesInProgress() const { return mGetMessagesInProgress; }
    void setMessagesInProgress(bool inProgress);

    void pause();
    void resume();

signals:
    void unreadCountChanged();
    void getConvosInProgressChanged();
    void startConvoForMembersOk(ConvoView convo);
    void startConvoForMembersFailed(QString error);
    void startConvoInProgressChanged();
    void leaveConvoOk();
    void getMessagesInProgressChanged();
    void getMessagesFailed(QString error);
    void getMessagesOk(QString cursor);
    void sendMessageProgress();
    void sendMessageFailed(QString error);
    void sendMessageOk();
    void deleteMessageFailed(QString error);
    void deleteMessageOk();
    void settingsFailed(QString error);
    void failure(QString error);

private:
    ATProto::ChatMaster* chatMaster();
    ATProto::PostMaster* postMaster();
    void setUnreadCount(int unread);
    void updateUnreadCount(const ATProto::ChatBskyConvo::ConvoListOutput& output);
    QString getLastReadMessageId(const ConvoView& convo) const;
    void updateMessages();
    void startMessagesUpdateTimer();
    void stopMessagesUpdateTimer();
    void startConvosUpdateTimer();
    void stopConvosUpdateTimer();
    bool isMessagesUpdating(const QString& convoId) const { return mConvoIdUpdatingMessages.contains(convoId); }
    void setMessagesUpdating(const QString& convoId, bool updating);
    void continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message, const QString& quoteUri, const QString& quoteCid);
    void continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message);

    std::unique_ptr<Presence> mPresence;
    ATProto::Client::Ptr& mBsky;
    std::unique_ptr<ATProto::ChatMaster> mChatMaster;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    const QString& mUserDid;
    ConvoListModel mConvoListModel;
    int mUnreadCount = 0;
    bool mGetConvosInProgress = false;
    bool mLoaded = false;
    std::unordered_map<QString, MessageListModel::Ptr> mMessageListModels; // convoId -> model
    std::unordered_set<QString> mConvoIdUpdatingMessages;
    bool mGetMessagesInProgress = false;
    bool mStartConvoInProgress = false;
    QTimer mMessagesUpdateTimer;
    QTimer mConvosUpdateTimer;
    QEnums::AllowIncomingChat mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
};

}
