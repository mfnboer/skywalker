// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "group_convo_member.h"

namespace Skywalker {

GroupConvoMember::GroupConvoMember(const ATProto::ChatBskyActor::GroupConvoMember::SharedPtr& member) :
    mMember(member)
{
}

}
