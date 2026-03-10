// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "sky_application.h"
#include <QNetworkReply>

namespace Skywalker {

SkyApplication::SkyApplication(int& argc, char** argv) :
    QGuiApplication(argc, argv),
    mMonitorThreadId(QThread::currentThreadId())
{
}

bool SkyApplication::notify(QObject* receiver, QEvent* event)
{
    if (mMonitorThreadId != QThread::currentThreadId())
        return QGuiApplication::notify(receiver, event);

    const auto objName = receiver->objectName();
    const auto className = receiver->metaObject() ? receiver->metaObject()->className() : "";
    auto* reply = dynamic_cast<QNetworkReply*>(receiver);
    const QUrl replyUrl = reply ? reply->url() : QUrl{};

    QElapsedTimer timer;
    timer.start();
    const bool ret = QGuiApplication::notify(receiver, event);
    const auto elapsed = timer.elapsed();

    if (elapsed > 16)
    {
        qDebug() << "Event:" << event->type() << "receiver:" << objName << className << "ms:" << elapsed;

        if (reply)
            qDebug() << "Network reply:" << replyUrl;
    }

    return ret;
}

}
