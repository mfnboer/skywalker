// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_profile.h"
#include "content_filter.h"
#include "group_convo_member.h"

namespace Skywalker {

ChatBasicProfile::ChatBasicProfile(const ATProto::ChatBskyActor::ProfileViewBasic& profile) :
    mBasicProfile(profile.mDid,
                    profile.mHandle,
                    profile.mDisplayName.value_or(""),
                    profile.mAvatar.value_or(""),
                    profile.mAssociated ? ProfileAssociated{profile.mAssociated} : ProfileAssociated{},
                    profile.mViewer ? ProfileViewerState{profile.mViewer} : ProfileViewerState{},
                    ContentFilter::getContentLabels(profile.mLabels)),
    mChatDisabled(profile.mChatDisabled)
{
    if (!profile.mKind)
    {
        mMemberKind = QEnums::CONVO_MEMBER_KIND_DIRECT;
    }
    else if (ATProto::holdsNonNull<ATProto::ChatBskyActor::DirectConvoMember::SharedPtr>(*profile.mKind))
    {
        mMemberKind = QEnums::CONVO_MEMBER_KIND_DIRECT;
    }
    else if (ATProto::holdsNonNull<ATProto::ChatBskyActor::GroupConvoMember::SharedPtr>(*profile.mKind))
    {
        mMemberKind = QEnums::CONVO_MEMBER_KIND_GROUP;
        auto member = std::get<ATProto::ChatBskyActor::GroupConvoMember::SharedPtr>(*profile.mKind);
        mGroupMember = std::make_shared<GroupConvoMember>(member);
    }
    else if (ATProto::holdsNonNull<ATProto::ChatBskyActor::PastGroupConvoMember::SharedPtr>(*profile.mKind))
    {
        mMemberKind = QEnums::CONVO_MEMBER_KIND_PAST_GROUP;
    }
    else if (ATProto::holdsNonNull<ATProto::UnknownVariant::SharedPtr>(*profile.mKind))
    {
        mMemberKind = QEnums::CONVO_MEMBER_KIND_UNKNOWN;
    }
    else
    {
        qWarning() << "Unexpected kind";
        mMemberKind = QEnums::CONVO_MEMBER_KIND_UNKNOWN;
    }
}

const GroupConvoMember& ChatBasicProfile::getGroupMember() const
{
    static const GroupConvoMember NULL_MEMBER;

    return mGroupMember ? *mGroupMember : NULL_MEMBER;
}

}
