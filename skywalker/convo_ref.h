// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>
#include <QtQmlIntegration>

namespace Skywalker {

class ConvoRef
{
    Q_GADGET
    Q_PROPERTY(QString did READ getDid FINAL)
    Q_PROPERTY(QString convoId READ getConvoId FINAL)
    QML_VALUE_TYPE(convoref)

public:
    ConvoRef() = default;
    explicit ConvoRef(const ATProto::ChatBskyConvo::ConvoRef::SharedPtr& ref);

    Q_INVOKABLE bool isNull() const { return mRef == nullptr; }
    QString getDid() const { return mRef ? mRef->mDid : ""; }
    QString getConvoId() const { return mRef ? mRef->mConvoId : ""; }

private:
    ATProto::ChatBskyConvo::ConvoRef::SharedPtr mRef;
};

}
