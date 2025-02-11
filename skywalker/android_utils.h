// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace Skywalker {

class AndroidUtils {

public:
    static bool checkPermission(const QString& permission);

private:
    static void setKeepScreenOn(bool keepOn);

    friend class DisplayUtils;
};

}
