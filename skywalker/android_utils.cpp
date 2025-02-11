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
