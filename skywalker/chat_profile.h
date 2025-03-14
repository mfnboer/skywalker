// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "profile.h"
#include <atproto/lib/lexicon/chat_bsky_actor.h>

namespace Skywalker {

class ChatBasicProfile
{
    Q_GADGET
    Q_PROPERTY(BasicProfile basicProfile READ getBasicProfile FINAL)
    Q_PROPERTY(bool chatDisabled READ isChatDisabled FINAL)
    QML_VALUE_TYPE(chatbasicprofile)

public:
    ChatBasicProfile() = default;
    explicit ChatBasicProfile(const ATProto::ChatBskyActor::ProfileViewBasic& profile);

    const BasicProfile& getBasicProfile() const { return mBasicProfile; }
    BasicProfile& getBasicProfile() { return mBasicProfile; }
    bool isChatDisabled() const { return mChatDisabled; }

    Q_INVOKABLE bool isNull() const { return mBasicProfile.isNull(); }

private:
    BasicProfile mBasicProfile;
    bool mChatDisabled = false;
};

using ChatBasicProfileList = QList<ChatBasicProfile>;

}
