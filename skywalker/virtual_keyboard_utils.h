// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <qqmlintegration.h>

namespace Skywalker {

class VirtualKeyboardUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit VirtualKeyboardUtils(QObject* parent = nullptr);

signals:
    void keyboardHeightChanged(int height);

private:
    static bool sInitialized;
};

}
