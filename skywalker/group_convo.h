// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class GroupConvo
{
    Q_GADGET
    Q_PROPERTY(QString name READ getName FINAL)
    QML_VALUE_TYPE(groupconvo)

public:
    GroupConvo() = default;
    explicit GroupConvo(const ATProto::ChatBskyConvo::GroupConvo::SharedPtr& groupConvo);

    Q_INVOKABLE bool isNull() const { return mGroupConvo == nullptr; }
    QString getName() const { return mGroupConvo ? mGroupConvo->mName : ""; }

private:
    ATProto::ChatBskyConvo::GroupConvo::SharedPtr mGroupConvo;
};

}
