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

void GraphUtils::follow(const QString& did)
{
    graphMaster()->follow(did,
        [this, presence=getPresence(), did](const auto& followingUri, const auto&){
            if (presence)
                emit followOk(followingUri);
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Follow failed:" << error;
            emit followFailed(error);
        });
}

void GraphUtils::unfollow(const QString& likeUri)
{
    graphMaster()->undo(likeUri,
        [this, presence=getPresence()]{

            if (presence)
                emit unfollowOk();
        },
        [this, presence=getPresence()](const QString& error){
            if (!presence)
                return;

            qDebug() << "Unfollow failed:" << error;
            emit unfollowFailed(error);
        });
}

}
