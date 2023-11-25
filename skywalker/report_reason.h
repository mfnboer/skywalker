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
    Q_PROPERTY(QString description READ getDescription FINAL)
    QML_VALUE_TYPE(reportreason)
public:
    ReportReason() = default;
    explicit ReportReason(QEnums::ReportReasonType reasonType) : mReasonType(reasonType) {}

    QEnums::ReportReasonType getType() const { return mReasonType; }
    QString getTitle() const { return ATProto::ComATProtoModeration::reasonTypeToTitle(
            ATProto::ComATProtoModeration::ReasonType(mReasonType)); }
    QString getDescription() const { return ATProto::ComATProtoModeration::reasonTypeToDescription(
            ATProto::ComATProtoModeration::ReasonType(mReasonType)); }

private:
    QEnums::ReportReasonType mReasonType = QEnums::REPORT_REASON_TYPE_NULL;
};

using ReportReasonList = QList<ReportReason>;

}

Q_DECLARE_METATYPE(Skywalker::ReportReason)
