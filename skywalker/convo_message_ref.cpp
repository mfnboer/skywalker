// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "convo_message_ref.h"

namespace Skywalker {

ConvoMessageRef::ConvoMessageRef(const ATProto::ChatBskyConvo::MessageRef::SharedPtr& ref) :
    mRef(ref)
{
}

}