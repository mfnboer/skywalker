// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneAccountStatus
{
    Q_GADGET
    Q_PROPERTY(QString source READ getSource FINAL)
    Q_PROPERTY(QEnums::OzoneAccountStanding standing READ getStanding FINAL)
    Q_PROPERTY(QString rawStanding READ getRawStanding FINAL)
    Q_PROPERTY(QString standingSymbol READ getStandingSymbol FINAL)
    Q_PROPERTY(bool hasActiveStrikes READ hasActiveStrikes FINAL)
    Q_PROPERTY(QDateTime updatedAt READ getUpdatedAt FINAL)
    QML_VALUE_TYPE(ozoneaccountstatus)

public:
    OzoneAccountStatus() = default;
    explicit OzoneAccountStatus(const ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr& status);

    Q_INVOKABLE bool isNull() const { return mStatus == nullptr; }
    QString getSource() const { return mStatus ? mStatus->mSrc : ""; }
    QEnums::OzoneAccountStanding getStanding() const { return mStatus ? (QEnums::OzoneAccountStanding)mStatus->mStanding.value() : QEnums::OZONE_ACCOUNT_STANDING_UNKNOWN; }
    QString getRawStanding() const { return mStatus ? mStatus->mStanding.toString() : QObject::tr("unknown"); }
    QString getStandingSymbol() const;
    bool hasActiveStrikes() const { return mStatus ? mStatus->mHasActiveStrikes : false; }
    QDateTime getUpdatedAt() const { return mStatus ? mStatus->mUpdatedAt : QDateTime{}; }

private:
    ATProto::ToolsOzoneInbox::GetAccountStatusOutput::SharedPtr mStatus;
};

}