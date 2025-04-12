// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/chat_bsky_convo.h>

namespace Skywalker {

class ReactionView
{
    Q_GADGET
    Q_PROPERTY(QString emoji READ getEmoji FINAL)
    Q_PROPERTY(QString senderDid READ getSenderDid FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    QML_VALUE_TYPE(reactionview)

public:
    using List = QList<ReactionView>;

    ReactionView() = default;
    explicit ReactionView(const ATProto::ChatBskyConvo::ReactionView::SharedPtr reaction);

    QString getEmoji() const;
    QString getSenderDid() const;
    QDateTime getCreatedAt() const;
    Q_INVOKABLE bool isNull() const { return mReaction == nullptr; }

private:
    ATProto::ChatBskyConvo::ReactionView::SharedPtr mReaction;
};

}
