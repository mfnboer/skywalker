// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "group_convo.h"

namespace Skywalker {

GroupConvo::GroupConvo(const ATProto::ChatBskyConvo::GroupConvo::SharedPtr& groupConvo) :
    mGroupConvo(groupConvo)
{
}

bool GroupConvo::isLocked() const
{
    const auto lockStatus = getLockStatus();

    switch (lockStatus)
    {
    case QEnums::CONVO_LOCK_STATUS_UNLOCKED:
        return false;
    case QEnums::CONVO_LOCK_STATUS_LOCKED:
    case QEnums::CONVO_LOCK_STATUS_LOCKED_PERMANENTLY:
        return true;
    case QEnums::CONVO_LOCK_STATUS_UNKNOWN:
        return false;
    }

    Q_ASSERT(false);
    qWarning() << "Unknown lock status:" << lockStatus;
    return false;
}

}
