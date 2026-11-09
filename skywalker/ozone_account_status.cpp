// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_account_status.h"

namespace Skywalker {

OzoneAccountStatus::OzoneAccountStatus(const ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr& status) :
    mStatus(status)
{
}

QString OzoneAccountStatus::getStandingSymbol() const
{
    const auto standing = getStanding();

    switch(standing)
    {
    case QEnums::OZONE_ACCOUNT_STANDING_GOOD:
        return "";
    case QEnums::OZONE_ACCOUNT_STANDING_WARNING:
        return "⚠️";
    case QEnums::OZONE_ACCOUNT_STANDING_AT_RISK:
        return "🛑";
    case QEnums::OZONE_ACCOUNT_STANDING_UNKNOWN:
        break;
    }

    return "❓";
}

}