// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneReportView
{
    Q_GADGET
    Q_PROPERTY(QString source READ getSource FINAL)
    Q_PROPERTY(int id READ getId FINAL)
    Q_PROPERTY(bool isRead READ isRead FINAL)
    Q_PROPERTY(QString reasonType READ getReasonType FINAL)
    Q_PROPERTY(QString reason READ getReason FINAL)
    Q_PROPERTY(QString lastActionTaken READ getLastActionTaken FINAL)
    Q_PROPERTY(QEnums::OzoneReportSubjectType subjectType READ getSubjectType FINAL)
    Q_PROPERTY(QVariant subject READ getSubjectType FINAL)
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(QDateTime updatedAt READ getUpdatedAt FINAL)
    QML_VALUE_TYPE(ozonereportview)

public:
    OzoneReportView() = default;
    explicit OzoneReportView(const ATProto::ToolsOzoneInbox::ReportView::SharedPtr& report);

    Q_INVOKABLE bool isNull() const { return mReport == nullptr; }
    QString getSource() const { return mReport ? mReport->mSrc : ""; }
    int getId() const { return mReport ? mReport->mId : -1; }
    bool isRead() const { return mReport ? mReport->mIsRead : false; }
    QString getReasonType() const { return mReport ? mReport->mReasonType : ""; }
    QString getReason() const { return mReport ? mReport->mReason.value_or("") : ""; }
    QString getLastActionTaken() const { return mReport ? mReport->mLastActionTaken.value_or("") : ""; }
    QEnums::OzoneReportSubjectType getSubjectType() const;
    QVariant getSubject() const;
    QEnums::OzoneReportStatus getStatus() const { return mReport ? (QEnums::OzoneReportStatus)mReport->mStatus.value() : QEnums::OZONE_REPORT_STATUS_UNKNOWN; }
    QDateTime getCreatedAt() const { return mReport ? mReport->mCreatedAt : QDateTime{}; }
    QDateTime getUpdatedAt() const { return mReport ? mReport->mUpdatedAt : QDateTime{}; }

private:
    ATProto::ToolsOzoneInbox::ReportView::SharedPtr mReport;
};

}