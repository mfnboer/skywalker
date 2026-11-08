// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneEnforcementView
{
    Q_GADGET
    Q_PROPERTY(QEnums::OzoneEnforcementState state READ getState FINAL)
    Q_PROPERTY(QDateTime expiresAt READ getExpiresAt FINAL)
    Q_PROPERTY(QStringList labels READ getLabels FINAL)
    QML_VALUE_TYPE(enforcementview)

public:
    OzoneEnforcementView() = default;
    explicit OzoneEnforcementView(const ATProto::ToolsOzoneInbox::EnforcementView::SharedPtr& enforcement);

    Q_INVOKABLE bool isNull() const { return mEnforcement == nullptr; }
    QEnums::OzoneEnforcementState getState() const { return mEnforcement ? (QEnums::OzoneEnforcementState)mEnforcement->mState.value() : QEnums::OZONE_ENFORCEMENT_STATE_UNKNOWN; }
    QDateTime getExpiresAt() const { return mEnforcement ? mEnforcement->mExpiresAt.value_or(QDateTime{}) : QDateTime{}; }
    QStringList getLabels() const { return mEnforcement ? mEnforcement->mLabels : QStringList{}; }

private:
    ATProto::ToolsOzoneInbox::EnforcementView::SharedPtr mEnforcement;
};

}