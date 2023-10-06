// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_utils.h"

namespace Skywalker {

GraphUtils::GraphUtils(QObject* parent) :
    QObject(parent),
    Presence()
{
}

ATProto::Client* GraphUtils::bskyClient()
{
    Q_ASSERT(mSkywalker);
    auto* client = mSkywalker->getBskyClient();
    Q_ASSERT(client);
    return client;
}

ATProto::GraphMaster* GraphUtils::graphMaster()
{
    if (!mGraphMaster)
    {
        Q_ASSERT(mSkywalker);
        Q_ASSERT(mSkywalker->getBskyClient());
        mGraphMaster = std::make_unique<ATProto::GraphMaster>(*mSkywalker->getBskyClient());
    }

    return mGraphMaster.get();
}

void GraphUtils::setSkywalker(Skywalker* skywalker)
{
    Q_ASSERT(skywalker);
    mSkywalker = skywalker;
    emit skywalkerChanged();
}

void GraphUtils::follow(const BasicProfile& profile)
{
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
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Follow failed:" << error;
            emit followFailed(error);
        });
}

void GraphUtils::unfollow(const QString& did, const QString& followingUri)
{
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
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Unfollow failed:" << error;
            emit unfollowFailed(error);
        });
}

void GraphUtils::block(const QString& did)
{
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
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Block failed:" << error;
            emit blockFailed(error);
        });
}

void GraphUtils::unblock(const QString& did, const QString& blockingUri)
{
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
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Unblock failed:" << error;
            emit unblockFailed(error);
        });
}

void GraphUtils::mute(const QString& did)
{
    bskyClient()->muteActor(did,
        [this, presence=getPresence()]{
            if (presence)
                emit muteOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Mute failed failed:" << error;
            emit muteFailed(error);
        });
}

void GraphUtils::unmute(const QString& did)
{
    bskyClient()->unmuteActor(did,
        [this, presence=getPresence()]{
            if (presence)
                emit unmuteOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Unmute failed failed:" << error;
            emit unmuteFailed(error);
        });
}

}
