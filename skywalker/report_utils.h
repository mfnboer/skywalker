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
    QML_ELEMENT

public:
    explicit ReportUtils(QObject* parent = nullptr);

    Q_INVOKABLE void reportAuthor(const QString& did, QEnums::ReportReasonType reasonType,
                                  const QString& details, const QString& labelerDid = "");
    Q_INVOKABLE void reportPostOrFeed(const QString& uri, const QString& cid, QEnums::ReportReasonType reasonType,
                                      const QString& details, const QString& labelerDid = "");

    Q_INVOKABLE static ReportReasonList getReportReasons(QEnums::ReportTarget target);

signals:
    void reportOk();
    void reportFailed(QString error);

private:
    static const ReportReasonList ACCOUNT_REASONS;
    static const ReportReasonList POST_REASONS;
    static const ReportReasonList FEED_REASONS;
    static const ReportReasonList LIST_REASONS;
};

}
