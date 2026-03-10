// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QThread>

namespace Skywalker {

class SkyApplication : public QGuiApplication
{
public:
    SkyApplication(int& argc, char** argv);
    void setMonitorThreadId();
    virtual bool notify(QObject* receiver, QEvent* event) override;

private:
    Qt::HANDLE mMonitorThreadId = nullptr;
};

}
