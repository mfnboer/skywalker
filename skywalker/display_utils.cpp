// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "display_utils.h"
#include "android_utils.h"
#include "skywalker.h"

namespace Skywalker {

int DisplayUtils::sScreenOnCount = 0;

DisplayUtils::DisplayUtils(QObject* parent) :
    WrappedSkywalker(parent)
{
}

ScopedHandle* DisplayUtils::keepScreenOn()
{
    if (sScreenOnCount == 0)
        AndroidUtils::setKeepScreenOn(true);

    ++sScreenOnCount;

    return new ScopedHandle([]{ disableScreenOn(); }, this);
}

void DisplayUtils::disableScreenOn()
{
    --sScreenOnCount;
    Q_ASSERT(sScreenOnCount >= 0);

    if (sScreenOnCount == 0)
        AndroidUtils::setKeepScreenOn(false);
}

bool DisplayUtils::sendAppToBackground()
{
    return AndroidUtils::sendAppToBackground();
}

void DisplayUtils::setNavigationBarColor(QColor color) const
{
    Q_ASSERT(mSkywalker);
    const auto displayMode = mSkywalker->getUserSettings()->getActiveDisplayMode();
    AndroidUtils::setNavigationBarColor(color, displayMode);
}

void DisplayUtils::setNavigationBarColorAndMode(QColor color, bool isLightMode)
{
    AndroidUtils::setNavigationBarColorAndMode(color, isLightMode);
}

int DisplayUtils::getNavigationBarSize(QEnums::InsetsSide side)
{
    return AndroidUtils::getNavigationBarSize(side);
}

int DisplayUtils::getStatusBarSize(QEnums::InsetsSide side)
{
    return AndroidUtils::getStatusBarSize(side);
}

void DisplayUtils::setStatusBarTransparent(bool transparent)
{
    AndroidUtils::setStatusBarTransparent(transparent);
}

}
