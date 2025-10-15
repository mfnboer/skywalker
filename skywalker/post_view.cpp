// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "post_view.h"

namespace Skywalker {

PostView::PostView(QObject* parent) :
    QObject(parent)
{
}

PostView::PostView(Post::Ptr post, QObject* parent) :
    QObject(parent),
    mPost(std::move(post)),
    mUri(mPost ? mPost->getUri() : "")
{
    Q_ASSERT(!mUri.isEmpty());
    Q_ASSERT(mPost);
}

PostView::PostView(const QString& uri, const QString& error, QObject* parent) :
    QObject(parent),
    mUri(uri),
    mError(error)
{
    Q_ASSERT(!mUri.isEmpty());
    Q_ASSERT(!mError.isEmpty());
}

const QString PostView::getCid() const
{
    return mPost ? mPost->getCid() : "";
}

QString PostView::getLikeUri() const
{
    if (mLikeUri)
        return *mLikeUri;

    return mPost ? mPost->getLikeUri() : "";
}

void PostView::setLikeUri(const QString& likeUri)
{
    if (!mLikeUri || *mLikeUri != mLikeUri)
    {
        mLikeUri = likeUri;
        emit likeUriChanged();
    }
}

bool PostView::isBookmarked() const
{
    if (mBookmarked)
        return *mBookmarked;

    return mPost ? mPost->isBookmarked() : false;
}

void PostView::setBookMarked(bool bookmarked)
{
    if (!mBookmarked || bookmarked != mBookmarked)
    {
        mBookmarked = bookmarked;
        emit bookmarkedChanged();
    }
}

bool PostView::isNotFound() const
{
    return mPost ? mPost->isNotFound() : false;
}

}
