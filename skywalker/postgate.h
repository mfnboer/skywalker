// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <atproto/lib/lexicon/app_bsky_feed.h>

namespace Skywalker {

class Postgate
{
    Q_GADGET
    Q_PROPERTY(QDateTime createdAt READ getCreatedAt FINAL)
    Q_PROPERTY(QString post READ getPost FINAL)
    Q_PROPERTY(QStringList detachedEmbeddingUris READ getDetachedEmbeddingUris FINAL)
    Q_PROPERTY(bool disabledEmbedding READ getDisabledEmbedding FINAL)
    QML_VALUE_TYPE(postgate)

public:
    Postgate() = default;
    explicit Postgate(const ATProto::AppBskyFeed::Postgate::SharedPtr& postgate) : mPostgate(postgate) {}

    Q_INVOKABLE bool isNull() const { return !mPostgate; }
    QDateTime getCreatedAt() const { return mPostgate ? mPostgate->mCreatedAt : QDateTime{}; }
    QString getPost() const { return mPostgate ? mPostgate->mPost : ""; }
    QStringList getDetachedEmbeddingUris() const { return mPostgate ? QStringList(mPostgate->mDetachedEmbeddingUris.begin(), mPostgate->mDetachedEmbeddingUris.end()) : QStringList{}; }
    bool getDisabledEmbedding() const { return mPostgate ? mPostgate->mDisableEmbedding : false; }

private:
    ATProto::AppBskyFeed::Postgate::SharedPtr mPostgate;
};

}
