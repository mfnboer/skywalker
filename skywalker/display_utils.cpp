// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "display_utils.h"
#include "android_utils.h"

namespace Skywalker {

int DisplayUtils::sScreenOnCount = 0;

DisplayUtils::DisplayUtils(QObject* parent) :
    QObject(parent)
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

}
