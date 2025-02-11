// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "scoped_handle.h"
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class DisplayUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DisplayUtils(QObject* parent = nullptr);

    Q_INVOKABLE ScopedHandle* keepScreenOn();

private:
    static void disableScreenOn();

    static int sScreenOnCount;
};

}
