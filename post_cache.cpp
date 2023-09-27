// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_cache.h"

namespace Skywalker {

PostCache::Entry::Entry(ATProto::AppBskyFeed::PostView::Ptr rawPostView, const Post& post) :
    mRawPostView(std::move(rawPostView)),
    mPost(post)
{
    Q_ASSERT(mRawPostView);
}

PostCache::PostCache() :
    mCache(500)
{}

void PostCache::put(ATProto::AppBskyFeed::PostView::Ptr rawPostView, const Post& post)
{
    auto* entry = new Entry(std::move(rawPostView), post);
    mCache.insert(post.getUri(), entry);
    qDebug() << "Cached:" << post.getUri() << "size:" << mCache.size();
}

const Post* PostCache::get(const QString& uri) const
{
    auto* entry = mCache[uri];
    return entry ? &entry->getPost() : nullptr;
}

bool PostCache::contains(const QString& uri) const
{
    return mCache.contains(uri);
}

}
