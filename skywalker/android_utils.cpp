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
    int rgb = color.rgba();
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/ScreenUtils", "setNavigationBarColor",
        "(IZ)V", (jint)rgb, (jboolean)isLightMode);
#else
    Q_UNUSED(color)
    Q_UNUSED(isLightMode)
#endif
}

int AndroidUtils::getNavigationBarSize(QEnums::InsetsSide side)
{
#ifdef Q_OS_ANDROID
    return (int)QJniObject::callStaticMethod<jint>(
        "com/gmail/mfnboer/ScreenUtils", "getNavigationBarSize", "(I)I", (jint)side);
#else
    Q_UNUSED(side)
    return 0;
#endif
}

void AndroidUtils::setStatusBarColor(QColor color, QEnums::DisplayMode displayMode)
{
    const bool isLightMode = (displayMode == QEnums::DISPLAY_MODE_LIGHT);
    setStatusBarColorAndMode(color, isLightMode);
}

void AndroidUtils::setStatusBarColorAndMode(QColor color, bool isLightMode)
{
#ifdef Q_OS_ANDROID
    int rgb = color.rgba();
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/ScreenUtils", "setStatusBarColor",
        "(IZ)V", (jint)rgb, (jboolean)isLightMode);
#else
    Q_UNUSED(color)
    Q_UNUSED(isLightMode)
#endif
}

int AndroidUtils::getStatusBarSize(QEnums::InsetsSide side)
{
#ifdef Q_OS_ANDROID
    return (int)QJniObject::callStaticMethod<jint>(
        "com/gmail/mfnboer/ScreenUtils", "getStatusBarSize", "(I)I", (jint)side);
#else
    Q_UNUSED(side)
    return 0;
#endif
}

void AndroidUtils::setStatusBarTransparent(bool transparent, QColor color, QEnums::DisplayMode displayMode)
{
    const bool isLightMode = (displayMode == QEnums::DISPLAY_MODE_LIGHT);
    setStatusBarTransparentAndMode(transparent, color, isLightMode);
}

void AndroidUtils::setStatusBarTransparentAndMode(bool transparent, QColor color, bool isLightMode)
{
#ifdef Q_OS_ANDROID
    int rgb = color.rgba();
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/ScreenUtils", "setStatusBarTransparent",
        "(ZIZ)V", (jboolean)transparent, (jint)rgb, (jboolean)isLightMode);
#else
    Q_UNUSED(transparent)
    Q_UNUSED(color)
    Q_UNUSED(isLightMode)
#endif
}

void AndroidUtils::setStatusBarLightMode(bool isLightMode)
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/ScreenUtils", "setStatusBarLightMode",
        "(Z)V", (jboolean)isLightMode);
#else
    Q_UNUSED(isLightMode)
#endif
}

void AndroidUtils::setKeepScreenOn(bool keepOn)
{
    qDebug() << "Keep screen on:" << keepOn;
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/ScreenUtils", "setKeepScreenOn", "(Z)V", (jboolean)keepOn);
#else
    Q_UNUSED(keepOn);
#endif
}

void AndroidUtils::installVirtualKeyboardListener()
{
    qDebug() << "Install virtual keyboard listener";
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/VirtualKeyboardListener", "installKeyboardListener");
#endif
}

bool AndroidUtils::translate(const QString& text)
{
    qDebug() << "Translate:" << text;
#if defined(Q_OS_ANDROID)
    auto jsText = QJniObject::fromString(text);
    return (bool)QJniObject::callStaticMethod<jboolean>(
        "com/gmail/mfnboer/Translate",
        "translate",
        "(Ljava/lang/String;)Z",
        jsText.object<jstring>());
#endif
    return false;
}

void AndroidUtils::showEmojiPicker(QEnums::DisplayMode displayMode)
{
    qDebug() << "Show emoji picker";
#if defined(Q_OS_ANDROID)
    const bool isLightMode = (displayMode == QEnums::DISPLAY_MODE_LIGHT);
    QJniObject::callStaticMethod<void>(
        "com/gmail/mfnboer/EmojiPickerDialog",
        "show", "(Z)V", (jboolean)isLightMode);
#else
    Q_UNUSED(displayMode)
#endif
}

void AndroidUtils::dismissEmojiPicker()
{
    qDebug() << "Dismiss emoji picker";
#if defined(Q_OS_ANDROID)
    QJniObject::callStaticMethod<void>("com/gmail/mfnboer/EmojiPickerDialog", "dismiss", "()V");
#endif
}

}
