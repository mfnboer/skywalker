// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "feed_utils.h"
#include "skywalker.h"

namespace Skywalker {

FeedUtils::FeedUtils(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
    connect(this, &WrappedSkywalker::skywalkerChanged, this, [this]{
        if (!mSkywalker)
            return;

        connect(mSkywalker, &Skywalker::bskyClientDeleted, this,
                [this]{
                    qDebug() << "Reset post master";
                    mPostMaster = nullptr;
                });
    });
}

ATProto::PostMaster* FeedUtils::postMaster()
{
    if (!mPostMaster)
    {
        auto* client = bskyClient();
        Q_ASSERT(client);

        if (client)
            mPostMaster = std::make_unique<ATProto::PostMaster>(*client);
        else
            qWarning() << "Bsky client not yet created";
    }

    return mPostMaster.get();
}

void FeedUtils::like(const QString& uri, const QString& cid)
{
    if (!postMaster())
        return;

    mSkywalker->makeLocalModelChange(
        [cid](LocalFeedModelChanges* model){
            model->updateLikeTransient(cid, true);
        });

    postMaster()->like(uri, cid,
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, likeUri](LocalFeedModelChanges* model){
                    model->updateLikeCountDelta(cid, 1);
                    model->updateLikeUri(cid, likeUri);
                    model->updateLikeTransient(cid, false);
                });

            emit likeOk(likeUri);
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg;

            mSkywalker->makeLocalModelChange(
                [cid](LocalFeedModelChanges* model){
                    model->updateLikeTransient(cid, false);
                });

            emit likeFailed(msg);
        });
}

void FeedUtils::undoLike(const QString& likeUri, const QString& cid)
{
    if (!postMaster())
        return;

    mSkywalker->makeLocalModelChange(
        [cid](LocalFeedModelChanges* model){
            model->updateLikeTransient(cid, true);
        });

    postMaster()->undo(likeUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalFeedModelChanges* model){
                    model->updateLikeCountDelta(cid, -1);
                    model->updateLikeUri(cid, "");
                    model->updateLikeTransient(cid, false);
                });

            emit undoLikeOk();
        },
        [this, presence=getPresence(), cid](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg;

            mSkywalker->makeLocalModelChange(
                [cid](LocalFeedModelChanges* model){
                    model->updateLikeTransient(cid, false);
                });

            emit undoLikeFailed(msg);
        });
}

void FeedUtils::hideFollowing(const QString& feedUri, bool hide)
{
    Q_ASSERT(mSkywalker);
    auto* settings = mSkywalker->getUserSettings();
    settings->setFeedHideFollowing(mSkywalker->getUserDid(), feedUri, hide);

    mSkywalker->makeLocalModelChange(
        [feedUri, hide](LocalFeedModelChanges* model){
            model->hideFollowing(feedUri, hide);
        });
}

}
