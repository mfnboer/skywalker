// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_profile.h"
#include <atproto/lib/lexicon/chat_bsky_convo.h>
#include <unordered_map>

namespace Skywalker {

class ConvoView
{
    Q_GADGET
    Q_PROPERTY(QString id READ getId FINAL)
    Q_PROPERTY(QString rev READ getRev FINAL)
    Q_PROPERTY(ChatBasicProfileList members READ getMembers FINAL)
    Q_PROPERTY(QString memberNames READ getMemberNames FINAL)
    Q_PROPERTY(bool muted READ isMuted FINAL)
    Q_PROPERTY(int unreadCount READ getUnreadCount FINAL)
    QML_VALUE_TYPE(convoview)
public:
    ConvoView() = default;
    explicit ConvoView(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& userDid);

    const QString& getId() const { return mId; }
    const QString& getRev() const { return mRev; }
    const ChatBasicProfileList& getMembers() const { return mMembers; }
    const QString getMemberNames() const { return mMemberNames.join(','); }
    bool isMuted() const { return mMuted; }
    int getUnreadCount() const { return mUnreadCount; }

    Q_INVOKABLE ChatBasicProfile getMember(const QString& did) const;

private:
    QString mId;
    QString mRev;
    ChatBasicProfileList mMembers; // all others than the user
    bool mMuted = false;
    int mUnreadCount = 0;
    // Last message
    QStringList mMemberNames;
    std::unordered_map<QString, int> mDidMemberMap; // DID -> index in mMembers
};

}
