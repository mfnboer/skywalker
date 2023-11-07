// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "bookmarks.h"

namespace Skywalker {

Bookmarks::Bookmarks(QObject* parent) :
    QObject(parent)
{
}

void Bookmarks::clear()
{
    mBookmarkedPostUris.clear();
    mPostUriIndex.clear();
    emit sizeChanged();
}

bool Bookmarks::addBookmark(const QString& postUri)
{
    if (isFull())
        return false;

    if (mPostUriIndex.count(postUri))
    {
        qDebug() << "Post already bookmarked:" << postUri;
        return true;
    }

    mPostUriIndex.insert(postUri);
    mBookmarkedPostUris.push_back(postUri);

    qDebug() << "Added bookmark:" << postUri;
    emit sizeChanged();
    return true;
}

void Bookmarks::removeBookmark(const QString& postUri)
{
    if (!mPostUriIndex.count(postUri))
    {
        qDebug() << "Post was not bookmarekd:" << postUri;
        return;
    }

    mPostUriIndex.erase(postUri);
    auto it = std::find(mBookmarkedPostUris.begin(), mBookmarkedPostUris.end(), postUri);
    Q_ASSERT(it != mBookmarkedPostUris.end());

    if (it != mBookmarkedPostUris.end())
        mBookmarkedPostUris.erase(it);

    qDebug() << "Removed bookmark:" << postUri;
    emit sizeChanged();
}

std::vector<QString> Bookmarks::getPage(int startIndex, int size) const
{
    std::vector<QString> page;
    page.reserve(size);

    // NOTE: new bookmarks are appended at the end!
    const int start = (int)mBookmarkedPostUris.size() - startIndex - 1;
    const int end = std::max(-1, start - size);

    for (int i = start; i > end; --i)
        page.push_back(mBookmarkedPostUris[i]);

    return page;
}

}
