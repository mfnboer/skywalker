// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "ozone_action_view.h"
#include "ozone_appeal_view.h"
#include "ozone_enforcement_view.h"

namespace Skywalker {

class IOzoneSubjectView
{
public:
    virtual ~IOzoneSubjectView() = default;

    virtual bool isNull() const = 0;
    virtual QString getSource() const = 0;
    virtual QString getSubjectUri() const = 0;
    virtual QEnums::OzoneReportSubjectType getSubjectType() const = 0;
    virtual QVariant getSubject() const;
    virtual OzoneEnforcementView getEnforcment() const = 0;
    virtual OzoneAppealView getAppeal() const = 0;
    virtual QList<QEnums::OzoneSubjectAction> getAvailableActions() const;
    virtual OzoneActionView getLatestAction() const = 0;
    virtual QDateTime getCreatedAt() const = 0;
    virtual QDateTime getUpdatedAt() const = 0;
    virtual bool isRead() const = 0;

private:
    virtual ATProto::ToolsOzoneInbox::SubjectView::SubjectType getAtprotoSubject() const = 0;
    virtual std::vector<ATProto::ToolsOzoneInbox::SubjectAction> getAtprotoActions() const = 0 ;
};

}
