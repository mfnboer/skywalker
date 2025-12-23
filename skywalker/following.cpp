// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "following.h"
#include <QDebug>

namespace Skywalker {

Following::Following(QObject* parent) : QObject(parent)
{
}

void Following::follow(const QString& did)
{
    qDebug() << "Follow:" << did;
    emit startedFollowing(did);
}

void Following::unfollow(const QString& did)
{
    qDebug() << "Unfollow:" << did;
    emit stoppedFollowing(did);
}

}
