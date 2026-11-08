// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_reports_summary.h"

namespace Skywalker {

OzoneReportsSummary::OzoneReportsSummary(const ATProto::ToolsOzoneInbox::ReportsSummary::SharedPtr& summary) :
    mSummary(summary)
{
}

}