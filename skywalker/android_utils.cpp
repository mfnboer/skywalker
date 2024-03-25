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
    auto checkResult = QtAndroidPrivate::checkPermission(permission);
    if (checkResult.result() != QtAndroidPrivate::Authorized)
    {
        qDebug() << "Permission check failed:" << permission;
        auto requestResult = QtAndroidPrivate::requestPermission(permission);

        if (requestResult.result() != QtAndroidPrivate::Authorized)
        {
            qWarning() << "No permission:" << permission;
            return false;
        }
    }

    return true;
}
#endif

}
