// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "local_post_model_changes.h"

namespace Skywalker {

const LocalPostModelChanges::Change* LocalPostModelChanges::getChange(const QString& cid) const
{
    auto it = mChanges.find(cid);
    return it != mChanges.end() ? &it->second : nullptr;
}

LocalPostModelChanges::Change& LocalPostModelChanges::getChangeForUpdate(const QString& cid)
{
    return mChanges[cid];
}

void LocalPostModelChanges::clear()
{
    mChanges.clear();
}

}
