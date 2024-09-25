// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "network_utils.h"
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QOperatingSystemVersion>
#include <QtCore/private/qandroidextras_p.h>
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

}
