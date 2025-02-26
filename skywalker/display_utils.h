// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "scoped_handle.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class DisplayUtils : public WrappedSkywalker
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DisplayUtils(QObject* parent = nullptr);

    Q_INVOKABLE ScopedHandle* keepScreenOn();
    Q_INVOKABLE static bool sendAppToBackground();
    Q_INVOKABLE void setNavigationBarColor(QColor color) const;
    Q_INVOKABLE static void setNavigationBarColorAndMode(QColor color, bool isLightMode);
    Q_INVOKABLE static int getNavigationBarSize(QEnums::InsetsSide side);
    Q_INVOKABLE void setStatusBarColor(QColor color) const;
    Q_INVOKABLE static void setStatusBarColorAndMode(QColor color, bool isLightMode);
    Q_INVOKABLE static int getStatusBarSize(QEnums::InsetsSide side);
    Q_INVOKABLE void setStatusBarTransparent(bool transparent, QColor color);
    Q_INVOKABLE static void setStatusBarTransparentAndMode(bool transparent, QColor color, bool isLightMode);

private:
    static void disableScreenOn();

    static int sScreenOnCount;
};

}
