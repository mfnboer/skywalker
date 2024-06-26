// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/client.h>
#include <atproto/lib/plc_directory_client.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class Skywalker;

class WrappedSkywalker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    QML_ELEMENT

public:
    explicit WrappedSkywalker(QObject* parent = nullptr);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

signals:
    void skywalkerChanged();

protected:
    ATProto::Client* bskyClient();
    ATProto::PlcDirectoryClient& plcDirectory();
    Skywalker* mSkywalker = nullptr;
};

}
