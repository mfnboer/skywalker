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

signals:
    void likeOk();
    void likeFailed(QString error);
    void undoLikeOk();
    void undoLikeFailed(QString error);

private:
    ATProto::PostMaster* postMaster();

    std::unique_ptr<ATProto::PostMaster> mPostMaster;
};

}
