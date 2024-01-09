// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "enums.h"
#include "presence.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/graph_master.h>

namespace Skywalker {

class GraphUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GraphUtils(QObject* parent = nullptr);

    Q_INVOKABLE void follow(const BasicProfile& profile);
    Q_INVOKABLE void unfollow(const QString& did, const QString& followingUri);
    Q_INVOKABLE void block(const QString& did);
    Q_INVOKABLE void unblock(const QString& did, const QString& blockingUri);
    Q_INVOKABLE void mute(const QString& did);
    Q_INVOKABLE void unmute(const QString& did);

    // avatarImgSource must be a 'file://' or 'image://' reference.
    Q_INVOKABLE void createList(const QEnums::ListPurpose purpose, const QString& name,
                                const QString& description, const QString& avatarImgSource);

signals:
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
    void createListProgress(QString msg);
    void createListOk(QString uri, QString cid);
    void createListFailed(QString error);

private:
    void continueCreateList(const QEnums::ListPurpose purpose, const QString& name,
                    const QString& description, ATProto::Blob::Ptr blob);

    ATProto::GraphMaster* graphMaster();
    std::unique_ptr<ATProto::GraphMaster> mGraphMaster;
};

}
