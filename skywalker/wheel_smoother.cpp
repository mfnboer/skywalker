// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "wheel_smoother.h"
#include <QCoreApplication>
#include <QPointer>
#include <QQuickWindow>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace Skywalker {

namespace {
constexpr int FRAME_MS = 16;
// Fraction of the remaining distance applied each frame
constexpr double STEP_FRACTION = 0.3;
// Limits in wheel angle units; one notch is 120.
constexpr double MAX_STEP = 240;
constexpr double MAX_BACKLOG = 120 * 30;
}

WheelSmoother::WheelSmoother(QObject* parent) :
    QObject(parent)
{
    mTimer.setInterval(FRAME_MS);
    connect(&mTimer, &QTimer::timeout, this, &WheelSmoother::step);
}

bool WheelSmoother::eventFilter(QObject* watched, QEvent* event)
{
    if (mSending || event->type() != QEvent::Wheel)
        return false;

    auto* window = qobject_cast<QQuickWindow*>(watched);
    auto* wheel = static_cast<QWheelEvent*>(event);

    if (!window || wheel->source() != Qt::MouseEventNotSynthesized ||
        wheel->phase() != Qt::NoScrollPhase || !wheel->pixelDelta().isNull() ||
        wheel->modifiers() != Qt::NoModifier || wheel->angleDelta().isNull())
    {
        return false;
    }

    const QPointF delta = wheel->angleDelta();

    // A change of direction cancels the scroll in progress
    if ((mRemaining.y() * delta.y() < 0) || (mRemaining.x() * delta.x() < 0))
        mRemaining = {};

    mRemaining += delta;
    mRemaining.setX(std::clamp(mRemaining.x(), -MAX_BACKLOG, MAX_BACKLOG));
    mRemaining.setY(std::clamp(mRemaining.y(), -MAX_BACKLOG, MAX_BACKLOG));
    mTarget = window;
    mPos = wheel->position();
    mGlobalPos = wheel->globalPosition();
    mButtons = wheel->buttons();

    if (!mTimer.isActive())
    {
        step();
        mTimer.start();
    }

    return true;
}

void WheelSmoother::step()
{
    QPointer<QObject> target = mTarget;

    if (!target)
    {
        mRemaining = {};
        mTimer.stop();
        return;
    }

    auto part = [](double remaining) {
        if (std::abs(remaining) < 2.0)
            return remaining;

        return std::clamp(remaining * STEP_FRACTION, -MAX_STEP, MAX_STEP);
    };

    const QPoint angle(std::lround(part(mRemaining.x())), std::lround(part(mRemaining.y())));
    mRemaining -= QPointF(angle);

    if (std::abs(mRemaining.x()) < 1.0 && std::abs(mRemaining.y()) < 1.0)
    {
        mRemaining = {};
        mTimer.stop();
    }

    if (angle.isNull())
        return;

    QWheelEvent wheel(mPos, mGlobalPos, QPoint(), angle, mButtons, Qt::NoModifier,
                      Qt::NoScrollPhase, false, Qt::MouseEventSynthesizedByApplication);

    mSending = true;
    QCoreApplication::sendEvent(target, &wheel);
    mSending = false;
}

}
