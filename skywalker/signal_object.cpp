// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "signal_object.h"
#include <QFile>
#include <QDebug>

namespace Skywalker {

FileSignal::FileSignal(const QString& fileName) :
    SignalObject(),
    mFileName(fileName)
{
}

FileSignal::~FileSignal()
{
    if (isHandled())
        return;

    qDebug() << "Not handled, delete file:" << mFileName;
    QFile::remove(mFileName);
}

}
