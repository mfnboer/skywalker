// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include <QColor>

namespace Skywalker {

class AndroidUtils {

public:
    static bool checkPermission(const QString& permission);
    static bool sendAppToBackground();
    static void setNavigationBarColor(QColor color, QEnums::DisplayMode displayMode);
    static void setNavigationBarColorAndMode(QColor color, bool isLightMode);
    static int getNavigationBarSize(QEnums::InsetsSide side);
    static int getStatusBarSize(QEnums::InsetsSide side);
    static void setStatusBarTransparent(bool transparent);

private:
    static void setKeepScreenOn(bool keepOn);

    friend class DisplayUtils;
};

}
