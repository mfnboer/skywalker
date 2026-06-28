// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "join_link_view.h"
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class GroupConvo
{
    Q_GADGET
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(int memberCount READ getMemberCount FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(int memberLimit READ getMemberLimit FINAL)
    Q_PROPERTY(QEnums::ConvoLockStatus lockStatus READ getLockStatus FINAL)
    Q_PROPERTY(JoinLinkView joinLinkView READ getJoinLinkView FINAL)
    QML_VALUE_TYPE(groupconvo)

public:
    GroupConvo() = default;
    explicit GroupConvo(const ATProto::ChatBskyConvo::GroupConvo::SharedPtr& groupConvo);

    Q_INVOKABLE bool isNull() const { return mGroupConvo == nullptr; }
    QString getName() const { return mGroupConvo ? mGroupConvo->mName : ""; }
    int getMemberCount() const { return mGroupConvo ? mGroupConvo->mMemberCount : 0; }
    QDateTime getCreatedAt() const { return mGroupConvo ? mGroupConvo->mCreatedAt : QDateTime{}; }
    int getMemberLimit() const { return mGroupConvo ? mGroupConvo->mMemberLimit : 0; }
    QEnums::ConvoLockStatus getLockStatus() const { return mGroupConvo ? (QEnums::ConvoLockStatus)mGroupConvo->mLockStatus : QEnums::CONVO_LOCK_STATUS_UNLOCKED; }
    Q_INVOKABLE bool isLocked() const;
    JoinLinkView getJoinLinkView() const { return mGroupConvo ? JoinLinkView{mGroupConvo->mJoinLink} : JoinLinkView{}; }
    ATProto::ChatBskyConvo::GroupConvo::SharedPtr getATProtoGroupConvo() const { return mGroupConvo; };

private:
    ATProto::ChatBskyConvo::GroupConvo::SharedPtr mGroupConvo;
};

}
