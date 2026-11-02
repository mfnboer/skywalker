// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>
#include <QtQmlIntegration>

namespace Skywalker {

class ConvoMessageRef
{
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString convoId READ getConvoId FINAL)
    Q_PROPERTY(QString messageId READ getMessageId FINAL)
    QML_VALUE_TYPE(convomessageref)

public:
    ConvoMessageRef() = default;
    explicit ConvoMessageRef(const ATProto::ChatBskyConvo::MessageRef::SharedPtr& ref);

    Q_INVOKABLE bool isNull() const { return mRef == nullptr; }
    QString getDid() const { return mRef ? mRef->mDid : ""; }
    QString getConvoId() const { return mRef ? mRef->mConvoId : ""; }
    QString getMessageId() const { return mRef ? mRef->mMessageId : ""; }

private:
    ATProto::ChatBskyConvo::MessageRef::SharedPtr mRef;
};

}