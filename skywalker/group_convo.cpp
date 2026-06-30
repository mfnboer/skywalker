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

void GroupConvo::clearUnreadJoinRequestCount()
{
    if (!mGroupConvo)
        return;

    // Don't change the existing group convo. It may be used elsewhere.
    auto newGroupConvo = std::make_shared<ATProto::ChatBskyConvo::GroupConvo>(*mGroupConvo);
    newGroupConvo->mUnreadJoinRequestCount = 0;
    mGroupConvo = newGroupConvo;
}

void GroupConvo::decrementJoinRequestCount()
{
    if (!mGroupConvo)
        return;

    // Don't change the existing group convo. It may be used elsewhere.
    auto newGroupConvo = std::make_shared<ATProto::ChatBskyConvo::GroupConvo>(*mGroupConvo);

    if (newGroupConvo->mJoinRequestCount.value_or(0) > 0)
        --(*newGroupConvo->mJoinRequestCount);

    mGroupConvo = newGroupConvo;
}

}
