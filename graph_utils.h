// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "skywalker.h"
#include <atproto/lib/graph_master.h>
#include <QObject>
#include <QtQmlIntegration>

namespace Skywalker {

class GraphUtils : public QObject, public Presence
{
    Q_OBJECT
    Q_PROPERTY(Skywalker* skywalker READ getSkywalker WRITE setSkywalker NOTIFY skywalkerChanged FINAL REQUIRED)
    QML_ELEMENT

public:
    explicit GraphUtils(QObject* parent = nullptr);

    Q_INVOKABLE void follow(const QString& did);
    Q_INVOKABLE void unfollow(const QString& did, const QString& followingUri);
    Q_INVOKABLE void block(const QString& did);
    Q_INVOKABLE void unblock(const QString& did, const QString& blockingUri);
    Q_INVOKABLE void mute(const QString& did);
    Q_INVOKABLE void unmute(const QString& did);

    Skywalker* getSkywalker() const { return mSkywalker; }
    void setSkywalker(Skywalker* skywalker);

signals:
    void skywalkerChanged();
    void followOk(QString uri);
    void followFailed(QString error);
    void unfollowOk();
    void unfollowFailed(QString error);
    void blockOk(QString uri);
    void blockFailed(QString error);
    void unblockOk();
    void unblockFailed(QString error);
    void muteOk();
    void muteFailed(QString error);
    void unmuteOk();
    void unmuteFailed(QString error);

private:
    ATProto::Client* bskyClient();
    ATProto::GraphMaster* graphMaster();

    Skywalker* mSkywalker = nullptr;
    std::unique_ptr<ATProto::GraphMaster> mGraphMaster;
};

}
