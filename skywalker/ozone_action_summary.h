// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "content_label.h"
#include "ozone_policy_summary.h"

namespace Skywalker {

class OzoneActionSummary
{
    Q_GADGET
    Q_PROPERTY(QString subject READ getSubject FINAL)
    Q_PROPERTY(OzonePolicySummary::List policies READ getPolicies FINAL)
    Q_PROPERTY(ContentLabel::List labels READ getLabels FINAL)
    QML_VALUE_TYPE(ozoneactionsummary)

public:
    OzoneActionSummary() = default;
    explicit OzoneActionSummary(const QString& subject, const OzonePolicySummary::List& policies = {}, const ContentLabel::List& labels = {});

    Q_INVOKABLE bool isNull() const { return mSubject.isEmpty(); }
    const QString& getSubject() const { return mSubject; }
    const OzonePolicySummary::List& getPolicies() const { return mPolicies; }
    const ContentLabel::List& getLabels() const { return mLabels; }

private:
    QString mSubject;
    OzonePolicySummary::List mPolicies;
    ContentLabel::List mLabels;
};

}
