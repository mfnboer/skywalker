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
    clearPrivate();
    mDirty = false;
    emit sizeChanged();
}

void Bookmarks::clearPrivate()
{
    mBookmarkedPostUris.clear();
    mPostUriIndex.clear();
}

bool Bookmarks::addBookmark(const QString& postUri)
{
    if (!addBookmarkPrivate(postUri))
        return false;

    mDirty = true;
    emit sizeChanged();
    return true;
}

bool Bookmarks::addBookmarkPrivate(const QString& postUri)
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
    return true;
}

void Bookmarks::removeBookmark(const QString& postUri)
{
    if (!mPostUriIndex.count(postUri))
    {
        qDebug() << "Post was not bookmarked:" << postUri;
        return;
    }

    mPostUriIndex.erase(postUri);
    auto it = std::find(mBookmarkedPostUris.begin(), mBookmarkedPostUris.end(), postUri);
    Q_ASSERT(it != mBookmarkedPostUris.end());

    if (it != mBookmarkedPostUris.end())
        mBookmarkedPostUris.erase(it);

    qDebug() << "Removed bookmark:" << postUri;
    mDirty = true;
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

void Bookmarks::load(const UserSettings* userSettings)
{
    Q_ASSERT(userSettings);
    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return;
    }

    clearPrivate();
    const QStringList& uris = userSettings->getBookmarks(did);

    for (const auto& uri : uris)
        addBookmarkPrivate(uri);

    qDebug() << "Bookmarks loaded:" << size();
    mDirty = false;
    emit sizeChanged();
}

void Bookmarks::save(UserSettings* userSettings)
{
    Q_ASSERT(userSettings);

    if (!mDirty)
        return;

    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return;
    }

    QStringList uris;

    for (const auto& uri : mBookmarkedPostUris)
        uris.append(uri);

    userSettings->saveBookmarks(did, uris);
    qDebug() << "Bookmarks saved:" << uris.size();
    mDirty = false;
}

bool Bookmarks::noticeSeen(const UserSettings* userSettings) const
{
    Q_ASSERT(userSettings);
    return userSettings->getBookmarksNoticeSeen();
}

void Bookmarks::setNoticeSeen(UserSettings* userSettings, bool seen) const
{
    Q_ASSERT(userSettings);
    userSettings->setBookmarksNoticeSeen(seen);
}

}
