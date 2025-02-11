// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "scoped_handle.h"

namespace Skywalker {

ScopedHandle::ScopedHandle(const ReleaseFun& releaseFun, QObject* parent) :
    QObject(parent),
    mReleaseFun{releaseFun}
{
}

ScopedHandle::~ScopedHandle()
{
    if (mReleaseFun)
        mReleaseFun();
}

}
