// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "ozone_action_summary.h"

namespace Skywalker {

OzoneActionSummary::OzoneActionSummary(const QString& subject, const OzonePolicySummary::List& policies, const ContentLabel::List& labels) :
    mSubject(subject),
    mPolicies(policies),
    mLabels(labels)
{
}

}