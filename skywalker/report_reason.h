// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class ReportReason
{
    Q_GADGET
    Q_PROPERTY(QEnums::ReportReasonType type READ getType FINAL)
    Q_PROPERTY(QString title READ getTitle FINAL)
    QML_VALUE_TYPE(reportreason)

public:
    using List = QList<ReportReason>;

    ReportReason() = default;
    ReportReason(QEnums::ReportReasonType reasonType, const QString& title) :
        mReasonType(reasonType),
        mTitle(title)
    {}

    QEnums::ReportReasonType getType() const { return mReasonType; }
    QString getTitle() const { return mTitle; }

private:
    QEnums::ReportReasonType mReasonType = QEnums::REPORT_REASON_TYPE_NULL;
    QString mTitle;
};

}

Q_DECLARE_METATYPE(::Skywalker::ReportReason)
