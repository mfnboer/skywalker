// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_profile.h"
#include "content_filter.h"

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
}

}
