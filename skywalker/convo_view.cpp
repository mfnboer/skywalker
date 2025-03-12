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

}
