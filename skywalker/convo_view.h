// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_profile.h"
#include "group_convo.h"
#include "message_view.h"
#include "message_and_reaction_view.h"
#include <atproto/lib/lexicon/chat_bsky_convo.h>
#include <unordered_map>

namespace Skywalker {

class ConvoView
{
    Q_GADGET
    Q_PROPERTY(QString id READ getId FINAL)
    Q_PROPERTY(QString rev READ getRev FINAL)
    Q_PROPERTY(QEnums::ConvoKind kind READ getKind FINAL)
    Q_PROPERTY(ChatBasicProfileList members READ getMembers FINAL)
    Q_PROPERTY(QString memberNames READ getMemberNames FINAL)
    Q_PROPERTY(bool muted READ isMuted FINAL)
    Q_PROPERTY(QEnums::ConvoStatus status READ getStatus FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount FINAL)
    Q_PROPERTY(MessageView lastMessage READ getLastMessage FINAL)
    Q_PROPERTY(MessageAndReactionView lastReaction READ getLastReaction FINAL)
    Q_PROPERTY(QDateTime lastMessageDate READ getLastMessageDate FINAL)
    Q_PROPERTY(GroupConvo group READ getGroupConvo FINAL)
    Q_PROPERTY(QString title READ getTitle FINAL)
    QML_VALUE_TYPE(convoview)

public:
    ConvoView() = default;
    explicit ConvoView(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& userDid);

    Q_INVOKABLE bool isNull() const { return mId.isEmpty(); }
    const QString& getId() const { return mId; }
    const QString& getRev() const { return mRev; }
    const QString& getRevIncludingReactions() const;
    QEnums::ConvoKind getKind() const { return mKind; }

    // NOTE:
    // For direct convos, members does not contain the active user
    // For group convos, members contain all users
    const ChatBasicProfileList& getMembers() const { return mMembers; }
    QString getMemberNames() const { return mMemberNames.join(", "); }

    bool isMuted() const { return mMuted; }
    QEnums::ConvoStatus getStatus() const { return mStatus; }
    int getUnreadCount() const { return mUnreadCount; }
    const MessageView& getLastMessage() const { return mLastMessage; }
    const MessageAndReactionView& getLastReaction() const { return mLastReaction; }
    QDateTime getLastMessageDate() const;
    const GroupConvo& getGroupConvo() const { return mGroupConvo; }
    QString getTitle() const;

    Q_INVOKABLE ChatBasicProfile getMember(const QString& did) const;

    void setRev(const QString& rev) { mRev = rev; }
    void setStatus(QEnums::ConvoStatus status) { mStatus = status; }
    void clearUnreadCount() { mUnreadCount = 0; }
    bool updateMemberBlocked(const QString& did, const QString& blockingUri);

private:
    QString mId;
    QString mRev;
    QEnums::ConvoKind mKind = QEnums::CONVO_KIND_DIRECT;
    ChatBasicProfileList mMembers; // all others than the user
    bool mMuted = false;
    QEnums::ConvoStatus mStatus = QEnums::CONVO_STATUS_UNKNOWN;
    int mUnreadCount = 0;
    MessageView mLastMessage;
    MessageAndReactionView mLastReaction;
    QStringList mMemberNames;
    std::unordered_map<QString, int> mDidMemberMap; // DID -> index in mMembers
    GroupConvo mGroupConvo;
};

}
