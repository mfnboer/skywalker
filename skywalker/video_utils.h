// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"

namespace Skywalker {

class VideoUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT

public:
    explicit VideoUtils(QObject* parent = nullptr);

    Q_INVOKABLE static void transcodeVideo(const QString inputFileName);

signals:
    void transcodingOk(QString inputFileName, QString outputFileName);
    void transcodingFailed(QString inputFileName, QString outputFileName, QString error);
};

}
