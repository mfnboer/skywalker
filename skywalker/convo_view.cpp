// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "convo_view.h"

namespace Skywalker {

ConvoView::ConvoView(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& userDid) :
    mId(convo.mId),
    mRev(convo.mRev),
    mMuted(convo.mMuted),
    mStatus(convo.mStatus ? QEnums::ConvoStatus(*convo.mStatus) : QEnums::CONVO_STATUS_UNKNOWN),
    mUnreadCount(convo.mUnreadCount)
{    
    mMembers.reserve(convo.mMembers.size());
    mMemberNames.reserve(convo.mMembers.size());

    for (const auto& member : convo.mMembers)
    {
        if (member->mDid != userDid)
        {
            mMembers.emplaceBack(*member);
            mMemberNames.push_back(mMembers.back().getBasicProfile().getName());
            mDidMemberMap[member->mDid] = mMembers.size() - 1;
        }
    }

    if (convo.mLastMessage)
        mLastMessage = MessageView{*convo.mLastMessage};

    if (convo.mLastReaction)
        mLastReaction = MessageAndReactionView{*convo.mLastReaction};
}

ChatBasicProfile ConvoView::getMember(const QString& did) const
{
    auto it = mDidMemberMap.find(did);
    return it != mDidMemberMap.end() ? mMembers[it->second] : ChatBasicProfile{};
}

QDateTime ConvoView::getLastMessageDate() const
{
    return mLastMessage.isNull() ? QDateTime{} : mLastMessage.getSentAt();
}

bool ConvoView::updateMemberBlocked(const QString& did, const QString& blockingUri)
{
    auto it = mDidMemberMap.find(did);

    if (it == mDidMemberMap.end())
        return false;

    auto& chatProfile = mMembers[it->second];
    BasicProfile& profile = chatProfile.getBasicProfile();
    profile.getViewer().setBlocking(blockingUri);
    return true;
}

// The rev of a convo does not get updated when a new reaction is added.
// The rev of the message on which a reaction is give, is updated however!
const QString& ConvoView::getRevIncludingReactions() const
{
    if (mLastReaction.isNull())
        return mRev;

    const QString& lastReactionRev = mLastReaction.getMessageView().getRev();
    return std::max(mRev, lastReactionRev);
}

}
