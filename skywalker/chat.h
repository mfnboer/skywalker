// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "convo_list_model.h"
#include "follows_activity_store.h"
#include "message_list_model.h"
#include "presence.h"
#include "web_link.h"
#include <atproto/lib/chat_master.h>
#include <atproto/lib/client.h>
#include <atproto/lib/post_master.h>

namespace Skywalker {

class Chat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ConvoListModel* acceptedConvoListModel READ getAcceptedConvoListModel CONSTANT FINAL)
    Q_PROPERTY(ConvoListModel* requestConvoListModel READ getRequestConvoListModel CONSTANT FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount NOTIFY unreadCountChanged FINAL)
    Q_PROPERTY(bool startConvoInProgress READ isStartConvoInProgress NOTIFY startConvoInProgressChanged FINAL)
    Q_PROPERTY(bool acceptConvoInProgress READ isAcceptConvoInProgress NOTIFY acceptConvoInProgressChanged FINAL)
    Q_PROPERTY(bool leaveConvoInProgress READ isLeaveConvoInProgress NOTIFY leaveConvoInProgressChanged FINAL)
    Q_PROPERTY(bool getMessagesInProgress READ isGetMessagesInProgress NOTIFY getMessagesInProgressChanged FINAL)

public:
    explicit Chat(ATProto::Client::SharedPtr& bsky, const QString& mUserDid, FollowsActivityStore& followsActivityStore, QObject* parent = nullptr);

    void reset();
    void initSettings();
    void updateSettings(QEnums::AllowIncomingChat allowIncoming);
    QEnums::AllowIncomingChat getAllowIncomingChat() const { return mAllowIncomingChat; }
    QString getLastRev() const;

    Q_INVOKABLE void getAllConvos();
    Q_INVOKABLE void getConvos(QEnums::ConvoStatus status, const QString& cursor = "");
    Q_INVOKABLE void getConvosNextPage(QEnums::ConvoStatus status);
    void updateConvos(QEnums::ConvoStatus status);
    Q_INVOKABLE void startConvoForMembers(const QStringList& dids, const QString& msg = {});
    Q_INVOKABLE void startConvoForMember(const QString& did, const QString& msg = {});
    Q_INVOKABLE void acceptConvo(const ConvoView& convo);
    Q_INVOKABLE void leaveConvo(const QString& convoId);
    Q_INVOKABLE void muteConvo(const QString& convoId);
    Q_INVOKABLE void unmuteConvo(const QString& convoId);
    Q_INVOKABLE BasicProfileList getAllAcceptedConvoMembers() const { return mAcceptedConvoListModel.getAllConvoMembers(); }
    Q_INVOKABLE bool convosLoaded(QEnums::ConvoStatus status) const;
    bool convosLoaded() const;

    ConvoListModel* getAcceptedConvoListModel() { return &mAcceptedConvoListModel; }
    ConvoListModel* getRequestConvoListModel() { return &mRequestConvoListModel; }
    int getUnreadCount() const { return mUnreadCount; }

    bool isStartConvoInProgress() const { return mStartConvoInProgress; }
    void setStartConvoInProgress(bool inProgress);

    bool isAcceptConvoInProgress() const { return mAcceptConvoInProgress; }
    void setAcceptConvoInProgress(bool inProgress);

    bool isLeaveConvoInProgress() const { return mLeaveConvoInProgress; }
    void setLeaveConvoInProgress(bool inProgress);

    Q_INVOKABLE MessageListModel* getMessageListModel(const QString& convoId);
    Q_INVOKABLE void removeMessageListModel(const QString& convoId);
    Q_INVOKABLE bool messageConvoOpen() const { return !mMessageListModels.empty(); }

    Q_INVOKABLE void getMessages(const QString& convoId, const QString& cursor = "");
    Q_INVOKABLE void getMessagesNextPage(const QString& convoId);
    Q_INVOKABLE void updateMessages(const QString& convoId);
    Q_INVOKABLE void updateRead(const QString& convoId);

    Q_INVOKABLE void sendMessage(const QString& convoId, const QString& text,
                                 const QString& quoteUri, const QString& quoteCid,
                                 const WebLink::List& embeddedLinks);
    Q_INVOKABLE void deleteMessage(const QString& convoId, const QString& messageId);

    Q_INVOKABLE void addReaction(const QString& convoId, const QString& messageId, const QString& emoji);
    Q_INVOKABLE void deleteReaction(const QString& convoId, const QString& messageId, const QString& emoji);

    bool isGetMessagesInProgress() const { return mGetMessagesInProgress; }
    void setMessagesInProgress(bool inProgress);

    void updateBlockingUri(const QString& did, const QString& blockingUri);

    void pause();
    void resume();

signals:
    void unreadCountChanged();
    void startConvoForMembersOk(ConvoView convo, QString msg);
    void startConvoForMembersFailed(QString error);
    void startConvoInProgressChanged();
    void acceptConvoInProgressChanged();
    void acceptConvoOk(ConvoView convo);
    void leaveConvoInProgressChanged();
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
    ConvoListModel* getConvoListModel(QEnums::ConvoStatus status) const;
    void updateConvoInModel(const ATProto::ChatBskyConvo::ConvoView& convo);
    void updateTotalUnreadCount();
    void setUnreadCount(QEnums::ConvoStatus status, int unread);
    void updateUnreadCount(QEnums::ConvoStatus status, const ATProto::ChatBskyConvo::ConvoListOutput& output);
    QString getLastReadMessageId(const ConvoView& convo) const;
    QString getLastRevIncludingReactions(ConvoListModel* model, ATProto::ChatBskyConvo::ConvoView::List& convos);
    void updateMessages();
    void startMessagesUpdateTimer();
    void stopMessagesUpdateTimer();
    void startConvosUpdateTimer(QEnums::ConvoStatus status);
    void stopConvosUpdateTimer(QEnums::ConvoStatus status);
    bool isMessagesUpdating(const QString& convoId) const { return mConvoIdUpdatingMessages.contains(convoId); }
    void setMessagesUpdating(const QString& convoId, bool updating);
    void continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message, const QString& quoteUri, const QString& quoteCid);
    void continueSendMessage(const QString& convoId, ATProto::ChatBskyConvo::MessageInput::SharedPtr message);

    std::unique_ptr<Presence> mPresence;
    ATProto::Client::SharedPtr& mBsky;
    std::unique_ptr<ATProto::ChatMaster> mChatMaster;
    std::unique_ptr<ATProto::PostMaster> mPostMaster;
    const QString& mUserDid;
    FollowsActivityStore& mFollowsActivityStore;
    ConvoListModel mAcceptedConvoListModel;
    ConvoListModel mRequestConvoListModel;
    int mUnreadCount = 0;
    std::unordered_map<QString, MessageListModel::Ptr> mMessageListModels; // convoId -> model
    std::unordered_set<QString> mConvoIdUpdatingMessages;
    bool mGetMessagesInProgress = false;
    bool mStartConvoInProgress = false;
    bool mAcceptConvoInProgress = false;
    bool mLeaveConvoInProgress = false;
    QTimer mMessagesUpdateTimer;
    QTimer mAcceptedConvosUpdateTimer;
    QTimer mRequestConvosUpdateTimer;
    QEnums::AllowIncomingChat mAllowIncomingChat = QEnums::ALLOW_INCOMING_CHAT_FOLLOWING;
};

}
