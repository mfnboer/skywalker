// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <atproto/lib/lexicon/chat_bsky_actor.h>

namespace Skywalker {

class GroupConvoMember;

class ChatBasicProfile
{
    Q_MOC_INCLUDE("group_convo_member.h")

    Q_GADGET
    Q_PROPERTY(BasicProfile basicProfile READ getBasicProfile FINAL)
    Q_PROPERTY(bool chatDisabled READ isChatDisabled FINAL)
    Q_PROPERTY(QEnums::ConvoMemberKind memberKind READ getMemberKind FINAL)
    Q_PROPERTY(GroupConvoMember groupMember READ getGroupMember FINAL)
    QML_VALUE_TYPE(chatbasicprofile)

public:
    ChatBasicProfile() = default;
    explicit ChatBasicProfile(const ATProto::ChatBskyActor::ProfileViewBasic& profile);

    const BasicProfile& getBasicProfile() const { return mBasicProfile; }
    BasicProfile& getBasicProfile() { return mBasicProfile; }
    bool isChatDisabled() const { return mChatDisabled; }
    QEnums::ConvoMemberKind getMemberKind() const { return mMemberKind; }
    const GroupConvoMember& getGroupMember() const;

    Q_INVOKABLE bool isNull() const { return mBasicProfile.isNull(); }

private:
    BasicProfile mBasicProfile;
    bool mChatDisabled = false;
    QEnums::ConvoMemberKind mMemberKind = QEnums::CONVO_MEMBER_KIND_DIRECT;
    std::shared_ptr<GroupConvoMember> mGroupMember;
};

using ChatBasicProfileList = QList<ChatBasicProfile>;

}
