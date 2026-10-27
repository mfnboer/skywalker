// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QUrl>

namespace Skywalker::NetworkUtils {

int getBandwidthKbps();
bool isUnmetered();
bool hostNeedsHttp1(const QUrl& url);

}
