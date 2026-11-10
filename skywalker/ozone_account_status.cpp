// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_account_status.h"

namespace Skywalker {

OzoneAccountStatus::OzoneAccountStatus(const ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr& status) :
    mStatus(status)
{
}

QString OzoneAccountStatus::getRawStanding() const {
    return mStatus ? mStatus->mStanding.toString() : QObject::tr("unknown");
}

QString OzoneAccountStatus::getStandingSymbol() const
{
    const auto standing = getStanding();
    return QEnums::ozoneAccountStandingToSymbol(standing);
}

}