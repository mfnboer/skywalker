// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "post.h"
#include <deque>

namespace Skywalker {

class ThreadUnroller
{
public:
    static std::deque<Post> unrollThread(const std::deque<Post>& thread);

private:
};

}
