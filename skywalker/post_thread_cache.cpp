// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "post_thread_cache.h"

namespace Skywalker {

std::unique_ptr<PostThreadCache> PostThreadCache::sInstance;

PostThreadCache::PostThreadCache(QObject* parent) :
    WrappedSkywalker(parent)
{
}

PostThreadCache& PostThreadCache::instance()
{
    if (!sInstance)
        sInstance = std::unique_ptr<PostThreadCache>(new PostThreadCache);

    return *sInstance;
}

void PostThreadCache::put(const QString& postUri, bool isThread)
{
    Q_ASSERT(!postUri.isEmpty());
    if (postUri.isEmpty())
        return;

    mCache.insert(postUri, new bool(isThread));
    qDebug() << "Cache size:" << mCache.size();
}

void PostThreadCache::putPost(const QString& uri)
{
    qDebug() << "Put post:" << uri;

    if (contains(uri))
    {
        qDebug() << "Post already in cache:" << uri;
        return;
    }

    if (mFetchingUris.contains(uri))
        return;

    if (!bskyClient())
        return;

    mFetchingUris.insert(uri);

    bskyClient()->getPostThread(uri, 1, 0,
        [this, uri](auto thread){
            mFetchingUris.erase(uri);

            if (putThread(thread->mThread))
                emit postAdded(uri);
        },
        [this, uri](const QString& error, const QString& msg){
            qDebug() << "putPost failed:" << uri << error << " - " << msg;

            if (!mFailedUris.contains(uri))
            {
                mFetchingUris.erase(uri);
                mFailedUris.insert(uri);
            }
            else
            {
                qWarning() << "Failed to get post URI for the second time:" << uri << error << " - " << msg;
                // Do not remove from mFetchingUris, so we will not try to get it again
            }
        });
}

bool PostThreadCache::putThread(const ATProto::AppBskyFeed::ThreadElement::SharedPtr& thread)
{   
    if (thread->mType != ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
        return false;

    const auto threadViewPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(thread->mPost);

    for (const auto& reply : threadViewPost->mReplies)
    {
        if (reply->mType != ATProto::AppBskyFeed::PostElementType::THREAD_VIEW_POST)
            continue;

        const auto replyThreadViewPost = std::get<ATProto::AppBskyFeed::ThreadViewPost::SharedPtr>(reply->mPost);

        if (replyThreadViewPost->mPost->mAuthor->mDid == threadViewPost->mPost->mAuthor->mDid)
        {
            qDebug() << "Post is thread:" << threadViewPost->mPost->mUri;
            put(threadViewPost->mPost->mUri, true);
            return true;
        }
    }

    qDebug() << "Post is not a thread:" << threadViewPost->mPost->mUri;
    put(threadViewPost->mPost->mUri, false);
    return true;
}

const bool* PostThreadCache::getIsThread(const QString& postUri) const
{
    return mCache[postUri];
}

bool PostThreadCache::contains(const QString& postUri) const
{
    return getIsThread(postUri) != nullptr;
}

}
