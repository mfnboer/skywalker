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

    postMaster()->like(uri, cid,
        [this, presence=getPresence(), cid](const auto& likeUri, const auto&){
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid, likeUri](LocalFeedModelChanges* model){
                    model->updateLikeCountDelta(cid, 1);
                    model->updateLikeUri(cid, likeUri);
                });

            emit likeOk(likeUri);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Like failed:" << error << " - " << msg;
            emit likeFailed(msg);
        });
}

void FeedUtils::undoLike(const QString& likeUri, const QString& cid)
{
    if (!postMaster())
        return;

    postMaster()->undo(likeUri,
        [this, presence=getPresence(), cid]{
            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [cid](LocalFeedModelChanges* model){
                    model->updateLikeCountDelta(cid, -1);
                    model->updateLikeUri(cid, "");
                });

            emit undoLikeOk();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qDebug() << "Undo like failed:" << error << " - " << msg;
            emit undoLikeFailed(msg);
        });
}

}
