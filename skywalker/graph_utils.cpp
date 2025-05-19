// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_utils.h"
#include "chat.h"
#include "definitions.h"
#include "graph_listener.h"
#include "list_cache.h"
#include "photo_picker.h"
#include "skywalker.h"
#include <atproto/lib/at_uri.h>

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

void GraphUtils::block(const QString& did, QDateTime expiresAt)
{
    if (!graphMaster())
        return;

    graphMaster()->block(did,
        [this, presence=getPresence(), did, expiresAt](const auto& blockingUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did, blockingUri](LocalAuthorModelChanges* model){
                    model->updateBlockingUri(did, blockingUri);
                });

            mSkywalker->makeLocalModelChange(
                [did](LocalProfileChanges* model){
                    model->setLocallyBlocked(did, true);
                });

            mSkywalker->getChat()->updateBlockingUri(did, blockingUri);

            if (expiresAt.isValid())
            {
                auto* settings = mSkywalker->getUserSettings();
                settings->addBlockWithExpiry(mSkywalker->getUserDid(), UriWithExpiry{blockingUri, expiresAt});
            }

            emit blockOk(blockingUri, expiresAt);
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
        [this, presence=getPresence(), did, blockingUri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateBlockingUri(did, "");
                });

            mSkywalker->makeLocalModelChange(
                [did](LocalProfileChanges* model){
                    model->setLocallyBlocked(did, false);
                });

            mSkywalker->getChat()->updateBlockingUri(did, "");

            auto* settings = mSkywalker->getUserSettings();
            settings->removeBlockWithExpiry(mSkywalker->getUserDid(), blockingUri);

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
        [this, presence=getPresence(), did]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateMuted(did, true);
                });

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
        [this, presence=getPresence(), did]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [did](LocalAuthorModelChanges* model){
                    model->updateMuted(did, false);
                });

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
                const QString& description, const WebLink::List& embeddedLinks,
                const QString& avatarImgSource)
{
    if (!bskyClient())
        return;

    if (!avatarImgSource.isEmpty())
    {
        emit createListProgress(tr("Uploading avatar"));

        QByteArray blob;
        const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit createListFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), purpose, name, description, embeddedLinks](auto blob){
                if (!presence)
                    return;

                continueCreateList(purpose, name, description, embeddedLinks, std::move(blob));
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
        continueCreateList(purpose, name, description, embeddedLinks, nullptr);
    }
}

void GraphUtils::continueCreateList(const QEnums::ListPurpose purpose, const QString& name,
                        const QString& description,const WebLink::List& embeddedLinks,
                        ATProto::Blob::SharedPtr blob)
{
    emit createListProgress(tr("Creating list"));
    const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);

    graphMaster()->createList(ATProto::AppBskyGraph::ListPurpose(purpose), name, description, embeddedFacets, std::move(blob), {},
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

void GraphUtils::createListFromStarterPack(const StarterPackView& starterPack)
{
    qDebug() << "Create list from starter pack:" << starterPack.getName();

    graphMaster()->createList(ATProto::AppBskyGraph::ListPurpose::CURATE_LIST, starterPack.getName(), starterPack.getDescription(), {}, nullptr, {},
        [this, presence=getPresence(), starterPack](const QString& uri, const QString& cid){
            if (!presence)
                return;

            continueCreateListFromStarterPack(starterPack, uri, cid);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Create list failed:" << error << " - " << msg;
            emit createdListFromStarterPackFailed(msg);
        });
}

void GraphUtils::continueCreateListFromStarterPack(const StarterPackView& starterPack, const QString &listUri, const QString& listCid, int maxPages, const std::optional<QString> cursor)
{
    qDebug() << "Copy users from starter pack to list:" << starterPack.getName();

    if (maxPages <= 0)
    {
        qWarning() << "Max pages reached";
        emit createdListFromStarterPackOk(starterPack, listUri, listCid);
    }

    bskyClient()->getList(starterPack.getList().getUri(), 100, cursor,
        [this, presence=getPresence(), starterPack, listUri, listCid, maxPages](auto output){
            if (!presence)
                return;

            QStringList dids;

            for (const auto& item : output->mItems)
                dids.push_back(item->mSubject->mDid);

            if (dids.isEmpty())
            {
                qDebug() << "Empty starter pack:" << starterPack.getName();
                emit createdListFromStarterPackOk(starterPack, listUri, listCid);
                return;
            }

            qDebug() << "Users:" << dids.size();

            graphMaster()->batchAddUsersToList(listUri, dids,
                [this, presence=getPresence(), starterPack, listUri, listCid, maxPages, nextCursor=output->mCursor]{
                    if (!presence)
                        return;

                    qDebug() << "Users added to starter pack:" << starterPack.getName();

                    if (!nextCursor)
                    {
                        qDebug() << "All users added:" << starterPack.getName();
                        emit createdListFromStarterPackOk(starterPack, listUri, listCid);
                        return;
                    }

                    continueCreateListFromStarterPack(starterPack, listUri, listCid, maxPages - 1, nextCursor);
                },
                [this, presence=getPresence(), listUri](const QString& error, const QString& msg){
                    if (!presence)
                        return;

                    qDebug() << "Adding users from starter pack failed:" << error << " - " << msg;
                    deleteList(listUri);
                    emit createdListFromStarterPackFailed(msg);
                });
        },
        [this, presence=getPresence(), listUri](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Adding users from starter pack failed:" << error << " - " << msg;
            deleteList(listUri);
            emit createdListFromStarterPackFailed(msg);
        });
}

void GraphUtils::updateList(const QString& listUri, const QString& name,
                const QString& description, const WebLink::List& embeddedLinks,
                const QString& avatarImgSource, bool updateAvatar)
{
    if (!bskyClient())
        return;

    if (updateAvatar && !avatarImgSource.isEmpty())
    {
        emit updateListProgress(tr("Uploading avatar"));

        QByteArray blob;
        const auto [mimeType, imgSize] = PhotoPicker::createBlob(blob, avatarImgSource);

        if (blob.isEmpty())
        {
            emit updateListFailed(tr("Could not load avatar image"));
            return;
        }

        bskyClient()->uploadBlob(blob, mimeType,
            [this, presence=getPresence(), listUri, name, description, embeddedLinks](auto blob){
                if (!presence)
                    return;

                continueUpdateList(listUri, name, description, embeddedLinks, std::move(blob), true);
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
        continueUpdateList(listUri, name, description, embeddedLinks, nullptr, updateAvatar);
    }
}

void GraphUtils::continueUpdateList(const QString& listUri, const QString& name,
                        const QString& description, const WebLink::List& embeddedLinks,
                        ATProto::Blob::SharedPtr blob, bool updateAvatar)
{
    emit updateListProgress(tr("Updating list"));
    const auto embeddedFacets = WebLink::toFacetList(embeddedLinks);

    graphMaster()->updateList(listUri, name, description, embeddedFacets, std::move(blob), updateAvatar,
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
        [this, presence=getPresence(), listUri]{
            if (!presence)
                return;

            ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();
            if (listUri == mutedReposts.getListUri())
            {
                qDebug() << "Muted reposts list deleted!";
                mutedReposts.setListCreated(false);
            }

            const bool listHidden = mSkywalker->getTimelineHide()->hasList(listUri);
            emit GraphListener::instance().listDeleted(listUri);

            if (listHidden)
                unhideList(listUri);

            syncList(listUri, false);

            emit deleteListOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Delete list failed:" << error << " - " << msg;
            emit deleteListFailed(msg);
        });
}

ListViewBasic GraphUtils::getCachedListView(const QString& listUri)
{
    auto& listCache = ListCache::instance();
    const auto* list = listCache.get(listUri);

    if (list)
        return *list;

    if (!bskyClient())
        return {};

    listCache.putList(listUri,
        [this, presence=getPresence(), listUri](){
            if (!presence)
                return;

            const auto* list = ListCache::instance().get(listUri);
            Q_ASSERT(list);

            if (list)
                emit cachedList(*list);
        });

    return {};
}

void GraphUtils::getListView(const QString& listUri, bool viewPosts)
{
    if (!bskyClient())
        return;

    bskyClient()->getList(listUri, 1, {},
        [this, presence=getPresence(), viewPosts](auto output){
            if (!presence)
                return;

            emit getListOk(ListView(output->mList), viewPosts);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getListViewfailed:" << error << " - " << msg;
            emit getListFailed(msg);
        });
}

void GraphUtils::isListUser(const QString& listUri, const QString& did, int maxPages, const std::optional<QString> cursor)
{
    if (!bskyClient())
        return;

    if (maxPages <= 0)
    {
        qWarning() << "Max pages reached";
        emit isListUserOk(listUri, did, {});
        return;
    }

    bskyClient()->getList(listUri, 100, cursor,
        [this, presence=getPresence(), listUri, did, maxPages](auto output){
            if (!presence)
                return;

            for (const auto& item : output->mItems)
            {
                if (item->mSubject->mDid == did)
                {
                    qDebug() << "User:" << did << "is member of list:" << listUri << ", itemUri:" << item->mUri;
                    emit isListUserOk(listUri, did, item->mUri);
                    return;
                }
            }

            if (output->mCursor)
            {
                isListUser(listUri, did, maxPages - 1, output->mCursor);
            }
            else
            {
                emit isListUserOk(listUri, did, {});
            }
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "getListViewfailed:" << error << " - " << msg;
            emit isListUserFailed(msg);
        });
}

void GraphUtils::addListUser(const QString& listUri, const BasicProfile& profile)
{
    if (!graphMaster())
        return;

    graphMaster()->addUserToList(listUri, profile.getDid(),
        [this, presence=getPresence(), listUri, profile](const QString& itemUri, const QString& itemCid){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri, itemUri](LocalListModelChanges* model){
                    model->updateMemberListItemUri(listUri, itemUri);
                });

            ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();
            if (listUri == mutedReposts.getListUri())
            {
                qDebug() << "List item is a muted repost, list-uri:" << listUri
                         << "item-uri:" << itemUri << "did:" << profile.getDid();
                mutedReposts.add(profile, itemUri);

                mSkywalker->makeLocalModelChange(
                    [did=profile.getDid()](LocalAuthorModelChanges* model){
                        model->updateMutedReposts(did, true);
                    });

                emit muteRepostsOk();
            }

            emit GraphListener::instance().userAdded(listUri, profile, itemUri);
            emit addListUserOk(profile.getDid(), itemUri, itemCid);
        },
        [this, presence=getPresence(), listUri](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "addListUser:" << error << " - " << msg;
            ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();

            if (listUri == mutedReposts.getListUri())
            {
                qDebug() << "List item is a muted repost, list-uri:" << listUri;
                emit muteRepostsFailed(msg);
            }

            emit addListUserFailed(msg);
        });
}

void GraphUtils::removeListUser(const QString& listUri, const QString& listItemUri)
{
    if (!graphMaster())
        return;

    graphMaster()->undo(listItemUri,
        [this, presence=getPresence(), listUri, listItemUri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri](LocalListModelChanges* model){
                    model->updateMemberListItemUri(listUri, {});
                });

            ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();
            if (listUri == mutedReposts.getListUri())
            {
                qDebug() << "List item is a muted repost, list-uri:" << listUri << "item-uri:" << listItemUri;
                const auto* did = mutedReposts.getDidByListItemUri(listItemUri);

                if (did)
                {
                    qDebug() << "Remove muted repost:" << *did;
                    const QString didCopy = *did;
                    mutedReposts.remove(*did);

                    mSkywalker->makeLocalModelChange(
                        [didCopy](LocalAuthorModelChanges* model){
                            model->updateMutedReposts(didCopy, false);
                        });

                    emit unmuteRepostsOk();
                }
            }

            emit GraphListener::instance().userRemoved(listUri, listItemUri);
            emit removeListUserOk();
        },
        [this, presence=getPresence(), listUri](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Remove list user failed:" << error << " - " << msg;
            ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();

            if (listUri == mutedReposts.getListUri())
                emit unmuteRepostsFailed(msg);

            emit removeListUserFailed(msg);
        });
}

ListView GraphUtils::makeListView(const QString& uri, const QString& cid, const QString& name,
                    QEnums::ListPurpose purpose, const QString& avatar, const Profile& creator,
                    const QString& description, const WebLink::List& embeddedLinks)
{
    return ListView(uri, cid, name, ATProto::AppBskyGraph::ListPurpose(purpose),
                    avatar, creator, description, embeddedLinks);
}

void GraphUtils::blockList(const QString& listUri)
{
    if (!graphMaster())
        return;

    graphMaster()->listBlock(listUri,
        [this, presence=getPresence(), listUri](const auto& blockingUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri, blockingUri](LocalListModelChanges* model){
                    model->updateBlocked(listUri, blockingUri);
                });

            emit blockListOk(blockingUri);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Block list failed:" << error << " - " << msg;
            emit blockListFailed(msg);
        });
}

void GraphUtils::unblockList(const QString& listUri, const QString& blockingUri)
{
    if (!graphMaster())
        return;

    graphMaster()->undo(blockingUri,
        [this, presence=getPresence(), listUri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri](LocalListModelChanges* model){
                    model->updateBlocked(listUri, "");
                });

            emit unblockListOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unblock list failed:" << error << " - " << msg;
            emit unblockListFailed(msg);
        });
}

void GraphUtils::muteList(const QString& listUri)
{
    if (!bskyClient())
        return;

    bskyClient()->muteActorList(listUri,
        [this, presence=getPresence(), listUri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri](LocalListModelChanges* model){
                    model->updateMuted(listUri, true);
                });

            emit muteListOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Mute list failed failed:" << error << " - " << msg;
            emit muteListFailed(msg);
        });
}

void GraphUtils::unmuteList(const QString& listUri)
{
    if (!bskyClient())
        return;

    bskyClient()->unmuteActorList(listUri,
        [this, presence=getPresence(), listUri]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [listUri](LocalListModelChanges* model){
                    model->updateMuted(listUri, false);
                });

            emit unmuteListOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Unmute failed failed:" << error << " - " << msg;
            emit unmuteListFailed(msg);
        });
}

void GraphUtils::hideList(const QString& listUri)
{
    Q_ASSERT(mSkywalker);
    mSkywalker->getTimelineHide()->addList(listUri,
        [this, presence=getPresence(), listUri]{
            if (!presence)
                return;

            auto* timelineHide = mSkywalker->getTimelineHide();
            const auto listUris = timelineHide->getListUris();
            mSkywalker->getUserSettings()->setHideLists(mSkywalker->getUserDid(), listUris);

            mSkywalker->makeLocalModelChange(
                [listUri](LocalListModelChanges* model){
                    model->hideFromTimeline(listUri, true);
                });

            mSkywalker->makeLocalModelChange(
                [listUri](LocalAuthorModelChanges* model){
                    model->updateHideFromTimeline();
                });

            emit hideListOk();
        },
        [this](auto, const QString& msg){ emit hideListFailed(msg); });
}

void GraphUtils::unhideList(const QString& listUri)
{
    Q_ASSERT(mSkywalker);
    auto* timelineHide = mSkywalker->getTimelineHide();
    timelineHide->removeList(listUri);
    const auto listUris = timelineHide->getListUris();
    mSkywalker->getUserSettings()->setHideLists(mSkywalker->getUserDid(), listUris);

    mSkywalker->makeLocalModelChange(
        [listUri](LocalListModelChanges* model){
            model->hideFromTimeline(listUri, false);
        });

    mSkywalker->makeLocalModelChange(
        [listUri](LocalAuthorModelChanges* model){
            model->updateHideFromTimeline();
        });
}

void GraphUtils::syncList(const QString& listUri, bool sync)
{
    Q_ASSERT(mSkywalker);
    auto* settings = mSkywalker->getUserSettings();

    if (sync)
        settings->addSyncFeed(mSkywalker->getUserDid(), listUri);
    else
        settings->removeSyncFeed(mSkywalker->getUserDid(), listUri);

    mSkywalker->makeLocalModelChange(
        [listUri, sync](LocalListModelChanges* model){
            model->syncList(listUri, sync);
        });
}

void GraphUtils::hideReplies(const QString& listUri, bool hide)
{
    Q_ASSERT(mSkywalker);
    auto* settings = mSkywalker->getUserSettings();
    settings->setFeedHideReplies(mSkywalker->getUserDid(), listUri, hide);

    mSkywalker->makeLocalModelChange(
        [listUri, hide](LocalListModelChanges* model){
            model->hideReplies(listUri, hide);
        });
}

void GraphUtils::hideFollowing(const QString& listUri, bool hide)
{
    Q_ASSERT(mSkywalker);
    auto* settings = mSkywalker->getUserSettings();
    settings->setFeedHideFollowing(mSkywalker->getUserDid(), listUri, hide);

    mSkywalker->makeLocalModelChange(
        [listUri, hide](LocalListModelChanges* model){
            model->hideFollowing(listUri, hide);
        });
}

bool GraphUtils::areRepostsMuted(const QString& did) const
{
    Q_ASSERT(mSkywalker);
    return mSkywalker->getMutedReposts().contains(did);
}

void GraphUtils::muteReposts(const BasicProfile& profile)
{
    Q_ASSERT(mSkywalker);
    ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();

    if (!mutedReposts.isListCreated())
    {
        if (!graphMaster())
            return;

        graphMaster()->createList(ATProto::AppBskyGraph::ListPurpose::MOD_LIST,
            "Skywalker muted reposts",
            "Used by Skywalker to keep track of muted reposts. Do not delete.",
            {}, {}, RKEY_MUTED_REPOSTS,
            [this, presence=getPresence(), profile](const QString& uri, const QString&){
                if (!presence)
                    return;

                Q_ASSERT(uri == mSkywalker->getMutedReposts().getListUri());
                mSkywalker->getMutedReposts().setListCreated(true);
                addListUser(uri, profile);
            },
            [this, presence=getPresence()](const QString& error, const QString& msg){
                if (!presence)
                    return;

                qDebug() << "Create mute reposts list failed:" << error << " - " << msg;
                emit muteRepostsFailed(msg);
            });
    }
    else
    {
        addListUser(mutedReposts.getListUri(), profile);
    }
}

void GraphUtils::unmuteReposts(const QString& did)
{
    Q_ASSERT(mSkywalker);
    ProfileListItemStore& mutedReposts = mSkywalker->getMutedReposts();
    const QString* listItemUri = mutedReposts.getListItemUri(did);

    if (!listItemUri)
        return;

    removeListUser(mutedReposts.getListUri(), *listItemUri);
}

bool GraphUtils::isInternalList(const QString& listUri) const
{
    ATProto::ATUri atUri(listUri);
    return atUri.isValid() && atUri.getRkey() == RKEY_MUTED_REPOSTS;
}

}
