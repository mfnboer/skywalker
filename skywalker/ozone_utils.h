// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "profile.h"
#include "ozone_report_view.h"
#include "ozone_resolution_view.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class OzoneUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OzoneUtils(QObject* parent = nullptr);

    Q_INVOKABLE static QString actionToDescription(const QString& action);
    Q_INVOKABLE static QString subjectTypeToString(const OzoneReportView& report);
    Q_INVOKABLE static QString subjectToString(const OzoneReportView& report);

    Q_INVOKABLE void getReportList(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0,  const QString& cursor = {});
    Q_INVOKABLE void getReportListNextPage(int modelId, int limit = 25, int maxPages = 1, int minEntries = 0);
    Q_INVOKABLE void getReport(int reportId, const BasicProfile& profile);

signals:
    // resolution can be null
    void reportOk(OzoneReportView report, OzoneResolutionView resolution, BasicProfile profile);
    void reportFailed(int reportId, QString error);
};

}