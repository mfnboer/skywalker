// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "file_utils.h"
#include <QDateTime>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QOperatingSystemVersion>
#include <QtCore/private/qandroidextras_p.h>
#endif

namespace {

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

namespace Skywalker::FileUtils {

bool checkReadMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString READ_EXTERNAL_STORAGE = "android.permission.READ_EXTERNAL_STORAGE";
    static const QString READ_MEDIA_IMAGES = "android.permission.READ_MEDIA_IMAGES";
    static const QString READ_MEDIA_VISUAL_USER_SELECTED = "android.permission.READ_MEDIA_VISUAL_USER_SELECTED";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion > QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES) && checkPermission(READ_MEDIA_VISUAL_USER_SELECTED);

    if (osVersion >= QOperatingSystemVersion::Android13)
        return checkPermission(READ_MEDIA_IMAGES);

    return checkPermission(READ_EXTERNAL_STORAGE);
#else
    return true;
#endif
}

bool checkWriteMediaPermission()
{
#if defined(Q_OS_ANDROID)
    static const QString WRITE_EXTERNAL_STORAGE = "android.permission.WRITE_EXTERNAL_STORAGE";

    const auto osVersion = QOperatingSystemVersion::current();

    if (osVersion < QOperatingSystemVersion::Android11)
        return checkPermission(WRITE_EXTERNAL_STORAGE);
#endif
    return true;
}

int openContentUri(const QString& contentUri)
{
#if defined(Q_OS_ANDROID)
    QJniObject uri = QJniObject::fromString(contentUri);

    int fd = QJniObject::callStaticMethod<int>(
        "com/gmail/mfnboer/FileUtils",
        "openContentUriString",
        "(Ljava/lang/String;)I",
        uri.object<jstring>());

    return fd;
#else
    qWarning() << "Cannot handle content-URI:" << contentUri;
    return -1;
#endif
}

QString resolveContentUriToFile(const QString &contentUriString) {
#ifdef Q_OS_ANDROID
    QJniObject uri = QJniObject::fromString(contentUriString);

    // Call the Java method
    QJniObject result = QJniObject::callStaticObjectMethod(
        "com/gmail/mfnboer/FileUtils",
        "resolveContentUriToFile",
        "(Ljava/lang/String;)Ljava/lang/String;",
        uri.object<jstring>());

    if (!result.isValid())
    {
        qWarning() << "Could not resolve content-uri:" << contentUriString;
        return {};
    }

    return result.toString();
#else
    Q_UNUSED(contentUriString)
    return {};
#endif
}

QString createDateTimeName()
{
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
}

}
