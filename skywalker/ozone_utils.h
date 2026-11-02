// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class OzoneUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit OzoneUtils(QObject* parent = nullptr);

    Q_INVOKABLE void getReportList(int id, int limit = 25, int maxPages = 1, int minEntries = 0,  const QString& cursor = {});
    Q_INVOKABLE void getReportListNextPage(int id, int limit = 25, int maxPages = 1, int minEntries = 0);
};

}