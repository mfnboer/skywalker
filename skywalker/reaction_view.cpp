// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "reaction_view.h"

namespace Skywalker {

ReactionView::ReactionView(const ATProto::ChatBskyConvo::ReactionView::SharedPtr reaction) :
    mReaction(reaction)
{}

QString ReactionView::getEmoji() const
{
    return mReaction ? mReaction->mValue : "";
}

QString ReactionView::getSenderDid() const
{
    return mReaction ? mReaction->mSender->mDid : "";
}

QDateTime ReactionView::getCreatedAt() const
{
    return mReaction ? mReaction->mCreatedAt : QDateTime{};
}

}
