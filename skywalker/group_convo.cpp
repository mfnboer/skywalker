// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "group_convo.h"

namespace Skywalker {

GroupConvo::GroupConvo(const ATProto::ChatBskyConvo::GroupConvo::SharedPtr& groupConvo) :
    mGroupConvo(groupConvo)
{
}

}
