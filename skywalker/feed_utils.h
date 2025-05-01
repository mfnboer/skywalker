// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include "wrapped_skywalker.h"
#include <atproto/lib/post_master.h>

namespace Skywalker {

class FeedUtils : public WrappedSkywalker, public Presence
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit FeedUtils(QObject* parent = nullptr);

    Q_INVOKABLE void like(const QString& uri, const QString& cid);
    Q_INVOKABLE void undoLike(const QString& likeUri, const QString& cid);
    Q_INVOKABLE void hideFollowing(const QString& feedUri, bool hide);
    Q_INVOKABLE void showMoreLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext);
    Q_INVOKABLE void showLessLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext);

signals:
    void likeOk(QString likeUri);
    void likeFailed(QString error);
    void undoLikeOk();
    void undoLikeFailed(QString error);
    void interactionsSent();
    void failure(QString error);

private:
    ATProto::PostMaster* postMaster();

    std::unique_ptr<ATProto::PostMaster> mPostMaster;
};

}
