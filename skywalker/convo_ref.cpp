// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "convo_ref.h"

namespace Skywalker {

ConvoRef::ConvoRef(const ATProto::ChatBskyConvo::ConvoRef::SharedPtr& ref) :
    mRef(ref)
{
}

}