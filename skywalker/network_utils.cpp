// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "network_utils.h"
#include <QDebug>
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QOperatingSystemVersion>
#endif

namespace Skywalker::NetworkUtils {

int getBandwidthKbps()
{
#if defined(Q_OS_ANDROID)
    const int kbps = QJniObject::callStaticMethod<jint>(
        "com/gmail/mfnboer/NetworkUtils",
        "getBandwidthKbps", "()I");

    qDebug() << "Bandwidth:" << kbps << "kbps";
    return kbps;
#else
    return 10'000; // Use 10 mbps as default for Linux
#endif
}

bool isUnmetered()
{
#if defined(Q_OS_ANDROID)
    const bool unmetered = (bool)QJniObject::callStaticMethod<jboolean>(
        "com/gmail/mfnboer/NetworkUtils",
        "isUnmetered", "()Z");

    qDebug() << "Unmetered:" << unmetered;
    return unmetered;
#else
    return true;
#endif
}

}
