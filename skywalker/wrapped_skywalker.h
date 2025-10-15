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
    Q_PROPERTY(QString nonActiveUserDid READ getNonActiveUserDid WRITE setNonActiveUserDid NOTIFY nonActiveUserDidChanged FINAL)
    QML_ELEMENT

public:
    explicit WrappedSkywalker(QObject* parent = nullptr);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

    const QString& getNonActiveUserDid() const { return mNonActiveUserDid; }
    void setNonActiveUserDid(const QString& did);

signals:
    void skywalkerChanged();
    void nonActiveUserDidChanged();

protected:
    ATProto::Client* bskyClient();
    ATProto::PlcDirectoryClient& plcDirectory();
    Skywalker* mSkywalker = nullptr;

    // If this DID is set, then the bsky session (client) for this user
    // will be used, otherwise the active user session is used.
    QString mNonActiveUserDid;
};

}
