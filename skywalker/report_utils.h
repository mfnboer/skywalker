// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "report_category.h"
#include "report_reason.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class ReportUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    Q_PROPERTY(int REPORT_DETAILS_SIZE MEMBER REPORT_DETAILS_SIZE CONSTANT)
    QML_ELEMENT

public:
    static constexpr int REPORT_DETAILS_SIZE = 300;

    explicit ReportUtils(QObject* parent = nullptr);

    Q_INVOKABLE void reportAuthor(const QString& did, QEnums::ReportReasonType reasonType,
                                  const QString& details, const QString& labelerDid = "");
    Q_INVOKABLE void reportPostOrFeed(const QString& uri, const QString& cid, QEnums::ReportReasonType reasonType,
                                      const QString& details, const QString& labelerDid = "");
    Q_INVOKABLE void reportDirectMessage(const QString& did, const QString& convoId, const QString& messageId,
                                         QEnums::ReportReasonType reasonType, const QString& details);

    Q_INVOKABLE static ReportCategory::List getReportCategories();
    Q_INVOKABLE static ReportReason::List getReportReasons(QEnums::ReportCategoryType category);

signals:
    void reportOk();
    void reportFailed(QString error);
};

}
