// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "android_utils.h"
#include <qdebug.h>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace Skywalker {

bool AndroidUtils::checkPermission(const QString& permission)
{
#if defined(Q_OS_ANDROID)
    auto checkFuture = QtAndroidPrivate::checkPermission(permission);

    if (!checkFuture.isValid())
    {
        qWarning() << "Invalid check future";
        return false;
    }

    if (checkFuture.result() != QtAndroidPrivate::Authorized)
    {
        qDebug() << "Permission check failed:" << permission;
        auto requestFuture = QtAndroidPrivate::requestPermission(permission);

        if (!requestFuture.isValid())
        {
            qWarning() << "Invalid request future";
            return false;
        }

        if (requestFuture.result() != QtAndroidPrivate::Authorized)
        {
            qWarning() << "No permission:" << permission;
            return false;
        }
    }
#else
    Q_UNUSED(permission)
#endif
    return true;
}

bool AndroidUtils::sendAppToBackground()
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return false;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("goToBack", "()V");
    return true;
#else
    return false;
#endif
}

void AndroidUtils::setNavigationBarColor(QColor color, QEnums::DisplayMode displayMode)
{
    const bool isLightMode = (displayMode == QEnums::DISPLAY_MODE_LIGHT);
    setNavigationBarColorAndMode(color, isLightMode);
}

void AndroidUtils::setNavigationBarColorAndMode(QColor color, bool isLightMode)
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    int rgb = color.rgba();
    activity.callMethod<void>("setNavigationBarColor", "(IZ)V", (jint)rgb, (jboolean)isLightMode);
#else
    Q_UNUSED(color)
    Q_UNUSED(isLightMode)
#endif
}

int AndroidUtils::getNavigationBarSize(QEnums::InsetsSide side)
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return 0;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    return (int)activity.callMethod<jint>("getNavigationBarSize", "(I)I", (jint)side);
#else
    Q_UNUSED(side)
    return 0;
#endif
}

int AndroidUtils::getStatusBarSize(QEnums::InsetsSide side)
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return 0;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    return (int)activity.callMethod<jint>("getStatusBarSize", "(I)I", (jint)side);
#else
    Q_UNUSED(side)
    return 0;
#endif
}

void AndroidUtils::setStatusBarTransparent(bool transparent)
{
#ifdef Q_OS_ANDROID
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("setStatusBarTransparent", "(Z)V", (jboolean)transparent);
#else
    Q_UNUSED(transparent)
#endif
}

void AndroidUtils::setKeepScreenOn(bool keepOn)
{
    qDebug() << "Keep screen on:" << keepOn;
#if defined(Q_OS_ANDROID)
    if (!QNativeInterface::QAndroidApplication::isActivityContext())
    {
        qWarning() << "Cannot find Android activity";
        return;
    }

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("setKeepScreenOn", "(Z)V", (jboolean)keepOn);
#else
    Q_UNUSED(keepOn);
#endif
}

}
