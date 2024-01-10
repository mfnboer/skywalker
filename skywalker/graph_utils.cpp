// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_utils.h"
#include "photo_picker.h"

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

void GraphUtils::createList(const QEnums::ListPurpose purpose, const QString& name,
                const QString& description, const QString& avatarImgSource)
{
    if (!bskyClient())
        return;

    if (!avatarImgSource.isEmpty())
    {
        emit createListProgress(tr("Uploading avatar"));

        QByteArray blob;
        const QString mimeType = createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit createListFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), purpose, name, description](auto blob){
                if (!presence)
                    return;

                continueCreateList(purpose, name, description, std::move(blob));
            },
            [this, presence=getPresence()](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qDebug() << "Create list failed:" << error << " - " << msg;
                emit createListFailed(msg);
            });
    }
    else
    {
        continueCreateList(purpose, name, description, nullptr);
    }
}

void GraphUtils::continueCreateList(const QEnums::ListPurpose purpose, const QString& name,
                        const QString& description, ATProto::Blob::Ptr blob)
{
    emit createListProgress(tr("Creating list"));

    graphMaster()->createList(ATProto::AppBskyGraph::ListPurpose(purpose), name, description, std::move(blob),
        [this, presence=getPresence()](const QString& uri, const QString& cid){
            if (!presence)
                return;

            emit createListOk(uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Create list failed:" << error << " - " << msg;
            emit createListFailed(msg);
        });
}

void GraphUtils::updateList(const QString& listUri, const QString& name,
                const QString& description, const QString& avatarImgSource,
                bool updateAvatar)
{
    if (!bskyClient())
        return;

    if (updateAvatar && !avatarImgSource.isEmpty())
    {
        emit updateListProgress(tr("Uploading avatar"));

        QByteArray blob;
        const QString mimeType = createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit updateListFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), listUri, name, description](auto blob){
                if (!presence)
                    return;

                continueUpdateList(listUri, name, description, std::move(blob), true);
            },
            [this, presence=getPresence()](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qDebug() << "Update list failed:" << error << " - " << msg;
                emit updateListFailed(msg);
            });
    }
    else
    {
        continueUpdateList(listUri, name, description, nullptr, updateAvatar);
    }
}

void GraphUtils::continueUpdateList(const QString& listUri, const QString& name,
                        const QString& description, ATProto::Blob::Ptr blob, bool updateAvatar)
{
    emit updateListProgress(tr("Updating list"));

    graphMaster()->updateList(listUri, name, description, std::move(blob), updateAvatar,
        [this, presence=getPresence()](const QString& uri, const QString& cid){
            if (!presence)
                return;

            emit updateListOk(uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Update list failed:" << error << " - " << msg;
            emit updateListFailed(msg);
        });
}

void GraphUtils::deleteList(const QString& listUri)
{
    if (!graphMaster())
        return;

    graphMaster()->undo(listUri,
        [this, presence=getPresence()]{
            if (!presence)
                return;

            emit deleteListOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Delete list failed:" << error << " - " << msg;
            emit deleteListFailed(msg);
        });
}

void GraphUtils::getListView(const QString& listUri)
{
    if (!bskyClient())
        return;

    bskyClient()->getList(listUri, 1, {},
        [this, presence=getPresence()](auto output){
            if (!presence)
                return;

            ATProto::AppBskyGraph::ListView::SharedPtr sharedListView(output->mList.release());
            emit getListOk(ListView(sharedListView));
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getListViewfailed:" << error << " - " << msg;
            emit getListFailed(msg);
        });
}

void GraphUtils::addListUser(const QString& listUri, const QString& did)
{
    if (!graphMaster())
        return;

    graphMaster()->addUserToList(listUri, did,
        [this, presence=getPresence(), did](const QString& uri, const QString& cid){
            if (!presence)
                return;

            emit addListUserOk(did, uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addListUser:" << error << " - " << msg;
            emit addListUserFailed(msg);
        });
}

}
