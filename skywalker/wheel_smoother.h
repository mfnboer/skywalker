// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QTimer>

namespace Skywalker {

// Spreads mouse wheel notches over a few frames so scrolling is smooth, also when
// the wheel is spinning freely. Touchpad and modified (e.g. ctrl) wheel events are
// not touched.
class WheelSmoother : public QObject
{
    Q_OBJECT

public:
    explicit WheelSmoother(QObject* parent = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void step();

    QTimer mTimer;
    QObject* mTarget = nullptr;
    QPointF mPos;
    QPointF mGlobalPos;
    Qt::MouseButtons mButtons;
    QPointF mRemaining;
    bool mSending = false;
};

}
