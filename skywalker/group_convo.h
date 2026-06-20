// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class GroupConvo
{
    Q_GADGET
    Q_PROPERTY(QString name READ getName FINAL)
    Q_PROPERTY(int memberCount READ getMemberCount FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(int memberLimit READ getMemberLimit FINAL)
    QML_VALUE_TYPE(groupconvo)

public:
    GroupConvo() = default;
    explicit GroupConvo(const ATProto::ChatBskyConvo::GroupConvo::SharedPtr& groupConvo);

    Q_INVOKABLE bool isNull() const { return mGroupConvo == nullptr; }
    QString getName() const { return mGroupConvo ? mGroupConvo->mName : ""; }
    int getMemberCount() const { return mGroupConvo ? mGroupConvo->mMemberCount : 0; }
    QDateTime getCreatedAt() const { return mGroupConvo ? mGroupConvo->mCreatedAt : QDateTime{}; }
    int getMemberLimit() const { return mGroupConvo ? mGroupConvo->mMemberLimit : 0; }

private:
    ATProto::ChatBskyConvo::GroupConvo::SharedPtr mGroupConvo;
};

}
