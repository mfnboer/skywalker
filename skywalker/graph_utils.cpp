// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_utils.h"

namespace Skywalker {

GraphUtils::GraphUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    connect(this, &WrappedSkywalker::skywalkerChanged, this, [this]{
        if (!mSkywalker)
            return;

        connect(mSkywalker, &Skywalker::bskyClientDeleted, this,
                [this]{
                    qDebug() << "Reset graph master";
                    mGraphMaster = nullptr;
                });
    });
}

ATProto::GraphMaster* GraphUtils::graphMaster()
{
    if (!mGraphMaster)
    {
        auto* client = bskyClient();
        Q_ASSERT(client);

        if (client)
            mGraphMaster = std::make_unique<ATProto::GraphMaster>(*bskyClient());
        else
            qWarning() << "Bsky client not yet created";
    }

    return mGraphMaster.get();
}

void GraphUtils::follow(const BasicProfile& profile)
{
    if (!graphMaster())
        return;

    graphMaster()->follow(profile.getDid(),
        [this, presence=getPresence(), profile](const auto& followingUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did=profile.getDid(), followingUri](LocalAuthorModelChanges* model){
                    model->updateFollowingUri(did, followingUri);
                });

            mSkywalker->getUserFollows().add(profile);

            emit followOk(followingUri);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Follow failed:" << error << " - " << msg;
            emit followFailed(msg);
        });
}

void GraphUtils::unfollow(const QString& did, const QString& followingUri)
{
    if (!graphMaster())
        return;

    graphMaster()->undo(followingUri,
        [this, presence=getPresence(), did]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateFollowingUri(did, "");
                });

            mSkywalker->getUserFollows().remove(did);

            emit unfollowOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unfollow failed:" << error << " - " << msg;
            emit unfollowFailed(msg);
        });
}

void GraphUtils::block(const QString& did)
{
    if (!graphMaster())
        return;

    graphMaster()->block(did,
        [this, presence=getPresence(), did](const auto& blockingUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did, blockingUri](LocalAuthorModelChanges* model){
                    model->updateBlockingUri(did, blockingUri);
                });

            emit blockOk(blockingUri);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Block failed:" << error << " - " << msg;
            emit blockFailed(msg);
        });
}

void GraphUtils::unblock(const QString& did, const QString& blockingUri)
{
    if (!graphMaster())
        return;

    graphMaster()->undo(blockingUri,
        [this, presence=getPresence(), did]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateFollowingUri(did, "");
                });

            emit unblockOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unblock failed:" << error << " - " << msg;
            emit unblockFailed(msg);
        });
}

void GraphUtils::mute(const QString& did)
{
    if (!bskyClient())
        return;

    bskyClient()->muteActor(did,
        [this, presence=getPresence()]{
            if (presence)
                emit muteOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Mute failed failed:" << error << " - " << msg;
            emit muteFailed(msg);
        });
}

void GraphUtils::unmute(const QString& did)
{
    if (!bskyClient())
        return;

    bskyClient()->unmuteActor(did,
        [this, presence=getPresence()]{
            if (presence)
                emit unmuteOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unmute failed failed:" << error << " - " << msg;
            emit unmuteFailed(msg);
        });
}

}
