// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "ozone_account_preview.h"
#include "ozone_action_view.h"
#include "ozone_appeal_view.h"
#include "ozone_enforcement_view.h"
#include "ozone_subject_view_interface.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneSubjectViewDetail : public IOzoneSubjectView
{
    Q_GADGET
    Q_PROPERTY(QString source READ getSource FINAL)
    Q_PROPERTY(QString subjectUri READ getSubjectUri FINAL)
    Q_PROPERTY(QEnums::OzoneReportSubjectType subjectType READ getSubjectType FINAL)
    Q_PROPERTY(QVariant subject READ getSubject FINAL)
    Q_PROPERTY(RecordContent recordContent READ getRecordContent FINAL)
    Q_PROPERTY(OzoneAccountPreview account READ getAccount FINAL)
    Q_PROPERTY(OzoneEnforcementView enforcement READ getEnforcment FINAL)
    Q_PROPERTY(OzoneAppealView appeal READ getAppeal FINAL)
    Q_PROPERTY(QList<QEnums::OzoneSubjectAction> availableActions READ getAvailableActions FINAL)
    Q_PROPERTY(OzoneActionView::List actions READ getActions FINAL)
    Q_PROPERTY(OzoneActionView latestAction READ getLatestAction FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(QDateTime updatedAt READ getUpdatedAt FINAL)
    Q_PROPERTY(bool isRead READ isRead FINAL)
    QML_VALUE_TYPE(ozonesubjectviewdetail)

public:
    OzoneSubjectViewDetail() = default;
    explicit OzoneSubjectViewDetail(const ATProto::ToolsOzoneInbox::SubjectViewDetail::SharedPtr& subject);

    Q_INVOKABLE bool isNull() const override { return mSubject == nullptr; }
    QString getSource() const override { return mSubject ? mSubject->mSrc : ""; }
    QString getSubjectUri() const override;
    RecordContent getRecordContent() const;
    OzoneAccountPreview getAccount() const { return (mSubject && mSubject->mAccount) ? OzoneAccountPreview{mSubject->mAccount} : OzoneAccountPreview{}; }
    QEnums::OzoneReportSubjectType getSubjectType() const override;
    OzoneEnforcementView getEnforcment() const override { return mSubject ? OzoneEnforcementView{mSubject->mEnforcement} : OzoneEnforcementView{}; }
    OzoneAppealView getAppeal() const override { return (mSubject && mSubject->mAppeal) ? OzoneAppealView{mSubject->mAppeal} : OzoneAppealView{}; }
    OzoneActionView::List getActions() const { return mSubject ? OzoneActionView::makeList(mSubject->mActions) : OzoneActionView::List{}; }
    OzoneActionView getLatestAction() const override;
    QDateTime getCreatedAt() const override { return mSubject ? mSubject->mCreatedAt : QDateTime{}; }
    QDateTime getUpdatedAt() const override { return mSubject ? mSubject->mUpdatedAt : QDateTime{}; }
    bool isRead() const override { return mSubject ? mSubject->mIsRead : false; }

    ATProto::ToolsOzoneInbox::SubjectViewDetail::SharedPtr getAtprotoPtr() const { return mSubject; }

    Q_INVOKABLE bool canAppeal() const;

private:
    ATProto::ToolsOzoneInbox::SubjectView::SubjectType getAtprotoSubject() const override { return mSubject ? mSubject->mSubject : ATProto::ToolsOzoneInbox::SubjectView::SubjectType{}; }
    std::vector<ATProto::ToolsOzoneInbox::SubjectAction> getAtprotoActions() const override { return mSubject ? mSubject->mAvailableActions : std::vector<ATProto::ToolsOzoneInbox::SubjectAction>{}; }

    ATProto::ToolsOzoneInbox::SubjectViewDetail::SharedPtr mSubject;
};

}