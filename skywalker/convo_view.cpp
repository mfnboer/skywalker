// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "convo_view.h"
#include "group_convo_member.h"

namespace Skywalker {

ConvoView::ConvoView(const ATProto::ChatBskyConvo::ConvoView& convo, const QString& userDid) :
    mId(convo.mId),
    mRev(convo.mRev),
    mKind((convo.mKind && ATProto::holdsNonNull<ATProto::ChatBskyConvo::GroupConvo::SharedPtr>(*convo.mKind)) ? QEnums::CONVO_KIND_GROUP : QEnums::CONVO_KIND_DIRECT),
    mMuted(convo.mMuted),
    mStatus(convo.mStatus ? QEnums::ConvoStatus(*convo.mStatus) : QEnums::CONVO_STATUS_UNKNOWN),
    mUnreadCount(convo.mUnreadCount),
    mGroupConvo(mKind == QEnums::CONVO_KIND_GROUP ? std::get<ATProto::ChatBskyConvo::GroupConvo::SharedPtr>(*convo.mKind) : nullptr)
{
    Q_ASSERT(mKind == QEnums::CONVO_KIND_GROUP || !userDid.isEmpty());
    mMembers.reserve(convo.mMembers.size());
    mMemberNames.reserve(convo.mMembers.size());

    for (const auto& member : convo.mMembers)
    {
        if (member->mDid != userDid || mKind == QEnums::CONVO_KIND_GROUP)
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

ConvoView::ConvoView(const ATProto::ChatBskyGroup::JoinRequestConvoView& joinRequest) :
    mId(joinRequest.mConvoId),
    mKind(QEnums::CONVO_KIND_GROUP),
    mStatus(QEnums::CONVO_STATUS_REQUEST),
    mIsRequestToJoin(true)
{
    ChatBasicProfile profile(*joinRequest.mOwner);
    mMembers.emplaceBack(profile);
    const auto basicProfile = profile.getBasicProfile();
    mMemberNames.push_back(basicProfile.getName());
    mDidMemberMap[basicProfile.getDid()] = 0;

    auto group = std::make_shared<ATProto::ChatBskyConvo::GroupConvo>();
    group->mName = joinRequest.mName;
    group->mMemberLimit = joinRequest.mMemberLimit;
    group->mMemberCount = joinRequest.mMemberCount;

    if (joinRequest.mViewer && joinRequest.mViewer->mRequestedAt)
        group->mCreatedAt = *joinRequest.mViewer->mRequestedAt;

    mGroupConvo = GroupConvo(group);
}

ChatBasicProfile ConvoView::getOwner() const
{
    if (mKind != QEnums::CONVO_KIND_GROUP)
        return {};

    for (const auto& member : mMembers)
    {
        const auto& groupMember = member.getGroupMember();

        if (groupMember.getRole() == QEnums::CONVO_MEMBER_ROLE_OWNER)
            return member;
    }

    return {};
}

ChatBasicProfile ConvoView::getMember(const QString& did) const
{
    auto it = mDidMemberMap.find(did);
    return it != mDidMemberMap.end() ? mMembers[it->second] : ChatBasicProfile{};
}

QString ConvoView::getInviteLinkUrl() const
{
    if (mGroupConvo.isNull())
    {
        qWarning() << "Not a group convo:" << mId;;
        return {};
    }

    const auto joinLink = mGroupConvo.getJoinLinkView();

    if (joinLink.isNull())
    {
        qWarning() << "No join link:" << mId;
        return {};
    }

    const QString code = joinLink.getCode();

    if (code.isEmpty())
    {
        qWarning() << "No join code:" << mId;
        return {};
    }

    return QString("https://bsky.app/chat/%1").arg(mGroupConvo.getJoinLinkView().getCode());
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

QString ConvoView::getTitle() const
{
    if (mKind == QEnums::CONVO_KIND_GROUP && !mGroupConvo.getName().isEmpty())
        return mGroupConvo.getName();

    return getMemberNames();
}

QDateTime ConvoView::getJoinRequestedAt() const
{
    if (!isRequestToJoin())
        return {};

    return mGroupConvo.getCreatedAt();
}

}
