// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneAppealView
{
    Q_GADGET
    Q_PROPERTY(QEnums::OzoneAppealState state READ getState FINAL)
    Q_PROPERTY(QDateTime appealedAt READ getAppealedAt FINAL)
    Q_PROPERTY(QDateTime resolvedAt READ getResolvedAt FINAL)
    Q_PROPERTY(QEnums::OzoneAppealOutcome outcome READ getOutcome FINAL)
    Q_PROPERTY(QDateTime appealableUntil READ getAppealableUntil FINAL)
    QML_VALUE_TYPE(ozoneappealview)

public:
    OzoneAppealView() = default;
    explicit OzoneAppealView(const ATProto::ToolsOzoneInbox::AppealView::SharedPtr& appeal);

    Q_INVOKABLE bool isNull() const { return mAppeal == nullptr; }
    QEnums::OzoneAppealState getState() const { return mAppeal ? (QEnums::OzoneAppealState)mAppeal->mState.value() : QEnums::OZONE_APPEAL_STATE_UNKNOWN; }
    QDateTime getAppealedAt() const { return (mAppeal && mAppeal->mAppealedAt) ? *mAppeal->mAppealedAt : QDateTime{}; }
    QDateTime getResolvedAt() const { return (mAppeal && mAppeal->mResolvedAt) ? *mAppeal->mResolvedAt : QDateTime{}; }
    QEnums::OzoneAppealOutcome getOutcome() const { return (mAppeal && mAppeal->mOutcome) ? (QEnums::OzoneAppealOutcome)mAppeal->mOutcome->value() : QEnums::OZONE_APPEAL_OUTCOME_UNKNOWN; }
    QDateTime getAppealableUntil() const { return (mAppeal && mAppeal->mAppealableUntil) ? *mAppeal->mAppealableUntil : QDateTime{}; }

private:
    ATProto::ToolsOzoneInbox::AppealView::SharedPtr mAppeal;
};

}