// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace Skywalker::AndroidUtils {

#if defined(Q_OS_ANDROID)
bool checkPermission(const QString& permission);
#endif

}
