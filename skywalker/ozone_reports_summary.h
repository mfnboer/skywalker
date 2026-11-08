// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/tools_ozone_inbox.h>

namespace Skywalker {

class OzoneReportsSummary
{
    Q_GADGET
    Q_PROPERTY(QStringList reasonTypes READ getReasonTypes FINAL)
    Q_PROPERTY(QDateTime firstReportedOn READ getFirstReportedOn FINAL)
    Q_PROPERTY(QDateTime lastReportedOn READ getLastReportedOn FINAL)
    QML_VALUE_TYPE(ozonereportssummary)

public:
    OzoneReportsSummary() = default;
    explicit OzoneReportsSummary(const ATProto::ToolsOzoneInbox::ReportsSummary::SharedPtr& summary);

    Q_INVOKABLE bool isNull() const { return mSummary == nullptr; }
    QStringList getReasonTypes() const { return mSummary ? mSummary->mReasonTypes : QStringList{}; }
    QDateTime getFirstReportedOn() const { return mSummary ? mSummary->mFirstReportedOn : QDateTime {}; }
    QDateTime getLastReportedOn() const { return mSummary ? mSummary->mLastReportedOn : QDateTime {}; }

private:
    ATProto::ToolsOzoneInbox::ReportsSummary::SharedPtr mSummary;
};

}