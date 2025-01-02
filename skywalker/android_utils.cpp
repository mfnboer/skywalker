// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "android_utils.h"

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace Skywalker::AndroidUtils {

#if defined(Q_OS_ANDROID)
bool checkPermission(const QString& permission)
{
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

    return true;
}
#endif

}
