// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "message_view.h"

namespace Skywalker {

class MessageAndReactionView
{
    Q_GADGET
    Q_PROPERTY(MessageView message READ getMessageView FINAL)
    Q_PROPERTY(ReactionView reaction READ getReactionView FINAL)
    QML_VALUE_TYPE(messageandreactionview)

public:
    MessageAndReactionView() = default;
    explicit MessageAndReactionView(const ATProto::ChatBskyConvo::ConvoView::ReactionType& reaction);

    const MessageView& getMessageView() const { return mMessageView; }
    const ReactionView& getReactionView() const { return mReactionView; }
    Q_INVOKABLE bool isNull() const { return mMessageView.isNull() || mReactionView.isNull(); }

private:
    MessageView mMessageView;
    ReactionView mReactionView;
};

}
