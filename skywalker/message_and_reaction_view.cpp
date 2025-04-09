// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "message_and_reaction_view.h"

namespace Skywalker {

MessageAndReactionView::MessageAndReactionView(const ATProto::ChatBskyConvo::ConvoView::ReactionType& reaction)
{
    if (ATProto::isNullVariant(reaction))
    {
        qWarning() << "Unknown reaction type";
        return;
    }

    const auto* view = std::get_if<ATProto::ChatBskyConvo::MessageAndReactionView::SharedPtr>(&reaction);

    if (view)
    {
        mMessageView = MessageView((*view)->mMessageView);
        mReactionView = ReactionView((*view)->mReactionView);
        return;
    }

    Q_ASSERT(false);
    qWarning() << "Should not get here";
}

}
