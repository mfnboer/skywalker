// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "report_reason.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ReportUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(ReportReasonList reportReasons READ getReportReasons CONSTANT FINAL)
    QML_ELEMENT

public:
    explicit ReportUtils(QObject* parent = nullptr);

    Q_INVOKABLE void reportAuthor(const QString& did, QEnums::ReportReasonType reasonType, const QString& details);
    Q_INVOKABLE void reportPostOrFeed(const QString& uri, const QString& cid, QEnums::ReportReasonType reasonType, const QString& details);

    const ReportReasonList& getReportReasons() const { return mReportReasons; }

signals:
    void reportOk();
    void reportFailed(QString error);

private:
    ReportReasonList mReportReasons;
};

}
