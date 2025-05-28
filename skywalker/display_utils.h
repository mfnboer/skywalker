// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "scoped_handle.h"
#include "wrapped_skywalker.h"
#include <QColor>

namespace Skywalker {

class DisplayUtils : public WrappedSkywalker
{
    Q_OBJECT
    Q_PROPERTY(QColor statusBarColor READ getStatusBarColor NOTIFY statusBarColorChanged FINAL)
    QML_ELEMENT

public:
    explicit DisplayUtils(QObject* parent = nullptr);

    QColor getStatusBarColor() const { return mStatusBarColor; }

    Q_INVOKABLE ScopedHandle* keepScreenOn();
    Q_INVOKABLE static bool sendAppToBackground();
    Q_INVOKABLE void setNavigationBarColor(QColor color) const;
    Q_INVOKABLE static void setNavigationBarColorAndMode(QColor color, bool isLightMode);
    Q_INVOKABLE static int getNavigationBarSize(QEnums::InsetsSide side);
    Q_INVOKABLE void setStatusBarColor(QColor color);
    Q_INVOKABLE void setStatusBarColorAndMode(QColor color, bool isLightMode);
    Q_INVOKABLE static int getStatusBarSize(QEnums::InsetsSide side);
    Q_INVOKABLE void setStatusBarTransparent(bool transparent, QColor color);
    Q_INVOKABLE void setStatusBarTransparentAndMode(bool transparent, QColor color, bool isLightMode);
    Q_INVOKABLE void resetStatusBarLightMode();

signals:
    void statusBarColorChanged();

private:
    void setInternalStatusBarColor(QColor color);
    static void disableScreenOn();

    static int sScreenOnCount;
    QColor mStatusBarColor;
    bool mIsLightMode = false;
};

}
