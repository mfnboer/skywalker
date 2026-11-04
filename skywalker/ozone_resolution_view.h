// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneResolutionView
{
    Q_GADGET
    Q_PROPERTY(QEnums::OzoneResolutionOutcome outcome READ getOutcome FINAL)
    Q_PROPERTY(QString actionTaken READ getActionTaken FINAL)
    Q_PROPERTY(QDateTime resolvedAt READ getResolvedAt FINAL)
    QML_VALUE_TYPE(ozoneresolutionview)

public:
    OzoneResolutionView() = default;
    explicit OzoneResolutionView(const ATProto::ToolsOzoneInbox::ResolutionView::SharedPtr& resolution);

    Q_INVOKABLE bool isNull() const { return mResolution == nullptr; }
    QEnums::OzoneResolutionOutcome getOutcome() const { return mResolution ? (QEnums::OzoneResolutionOutcome)mResolution->mOutcome.value() : QEnums::OZONE_RESOLUTION_OUTCOME_UNKNOWN; }
    QString getActionTaken() const { return mResolution ? mResolution->mActionTaken.value_or("") : ""; }
    QDateTime getResolvedAt() const { return mResolution ? mResolution->mResolvedAt : QDateTime{}; }

private:
    ATProto::ToolsOzoneInbox::ResolutionView::SharedPtr mResolution;
};

}