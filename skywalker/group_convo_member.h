// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_profile.h"
#include "enums.h"
#include <atproto/lib/lexicon/chat_bsky_actor.h>

namespace Skywalker {

class GroupConvoMember
{
    Q_GADGET
    Q_PROPERTY(ChatBasicProfile addedBy READ getAddedBy FINAL)
    Q_PROPERTY(QEnums::ConvoMemberRole role READ getRole FINAL)
    QML_VALUE_TYPE(convogroupmember)

public:
    GroupConvoMember() = default;
    explicit GroupConvoMember(const ATProto::ChatBskyActor::GroupConvoMember::SharedPtr& member);

    Q_INVOKABLE bool isNull() const { return mMember == nullptr; }
    ChatBasicProfile getAddedBy() const { return mMember && mMember->mAddedBy ? ChatBasicProfile{*mMember->mAddedBy} : ChatBasicProfile{}; }
    QEnums::ConvoMemberRole getRole() const { return mMember ? QEnums::ConvoMemberRole(mMember->mRole) : QEnums::CONVO_MEMBER_ROLE_UNKNOWN; }

private:
    ATProto::ChatBskyActor::GroupConvoMember::SharedPtr mMember;
};

}
