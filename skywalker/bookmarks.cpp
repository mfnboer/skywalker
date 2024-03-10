// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "bookmarks.h"
#include "lexicon/lexicon.h"
#include "skywalker.h"
#include <atproto/lib/at_uri.h>

namespace Skywalker {

Bookmarks::Bookmarks(QObject* parent) :
    WrappedSkywalker(parent),
    Presence()
{
}

void Bookmarks::clear()
{
    qDebug() << "clear bookmarks";
    mBookmarkedPostUris.clear();
    mPostUriIndex.clear();
    mPostUriRecordUriMap.clear();
    emit sizeChanged();
}

bool Bookmarks::addBookmark(const QString& postUri)
{
    if (mPostUriIndex.count(postUri))
    {
        qDebug() << "Post already bookmarked:" << postUri;
        return true;
    }

    if (isFull())
        return false;
    
    Bookmark::Bookmark bookmark;
    bookmark.mUri = postUri;

    writeRecord(bookmark);
    return true;
}

bool Bookmarks::addBookmarkPrivate(const QString& postUri)
{
    if (mPostUriIndex.count(postUri))
    {
        qDebug() << "Post already bookmarked:" << postUri;
        return false;
    }

    mPostUriIndex.insert(postUri);
    mBookmarkedPostUris.push_back(postUri);

    qDebug() << "Added bookmark:" << postUri;
    return true;
}

void Bookmarks::removeBookmark(const QString& postUri)
{
    if (!mPostUriRecordUriMap.count(postUri))
    {
        qDebug() << "Post was not bookmarked:" << postUri;
        return;
    }

    deleteRecord(postUri);
}

std::vector<QString> Bookmarks::getPage(int startIndex, int size) const
{
    qDebug() << "Get page, start:" << startIndex << "size:" << size << "#bookmarks:" << mBookmarkedPostUris.size();
    std::vector<QString> page;
    page.reserve(size);

    // NOTE: new bookmarks are appended at the end!
    const int start = (int)mBookmarkedPostUris.size() - startIndex - 1;
    const int end = std::max(-1, start - size);

    for (int i = start; i > end; --i)
        page.push_back(mBookmarkedPostUris[i]);

    return page;
}

void Bookmarks::load()
{
    qDebug() << "Load bookmarks";
    clear();

    listRecords([this]{
        const QStringList uris = loadLegacy();

        if (uris.isEmpty())
        {
            qDebug() << "Bookmarks loaded.";
            removeLegacyBookmarks();
            emit bookmarksLoaded();
        }
        else
        {
            qDebug() << "Migrate legacy bookmarks to:" << Lexicon::COLLECTION_BOOKMARK;
            createRecords(uris, [this]{
                qDebug() << "Bookmarks migrated to:" << Lexicon::COLLECTION_BOOKMARK;
                removeLegacyBookmarks();

                // Reload all from repo to create the post-uri -> record-uri mapping
                load();
            });
        }
    });
}

void Bookmarks::removeLegacyBookmarks()
{
    qDebug() << "Remove legacy bookmarks";
    auto* userSettings = mSkywalker->getUserSettings();
    const QString did = userSettings->getActiveUserDid();
    userSettings->removeBookmarks(did);
}

QStringList Bookmarks::loadLegacy()
{
    const auto* userSettings = mSkywalker->getUserSettings();
    const QString did = userSettings->getActiveUserDid();

    if (did.isEmpty())
    {
        qDebug() << "No active user";
        return {};
    }

    const QStringList& uris = userSettings->getBookmarks(did);
    QStringList newUris;

    for (const auto& uri : uris)
    {
        if (addBookmarkPrivate(uri))
            newUris.push_back(uri);
    }

    if (!newUris.isEmpty())
    {
        qDebug() << "Legacy bookmarks loaded:" << size();
        emit sizeChanged();
    }
    else
    {
        qDebug() << "No legacy bookmarks";
    }

    return newUris;
}

void Bookmarks::writeRecord(const Bookmark::Bookmark& bookmark)
{
    if (!bskyClient())
        return;

    const QString& repo = mSkywalker->getUserDid();
    const auto json = bookmark.toJson();

    bskyClient()->createRecord(repo, Lexicon::COLLECTION_BOOKMARK, {}, json, false,
        [this, bookmark, presence=getPresence()](auto ref) {
            if (!presence)
                return;

            addBookmarkPrivate(bookmark.mUri);
            mPostUriRecordUriMap[bookmark.mUri] = ref->mUri;
            emit sizeChanged();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed save bookmark:" << error << "-" << msg;
            mSkywalker->showStatusMessage(tr("Could not bookmark: %1").arg(msg), QEnums::STATUS_LEVEL_ERROR);
        });
}

void Bookmarks::deleteRecord(const QString& postUri)
{
    const auto& recordUri = mPostUriRecordUriMap[postUri];
    ATProto::ATUri atUri(recordUri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << recordUri;
        return;
    }

    if (!bskyClient())
        return;

    bskyClient()->deleteRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(),
        [this, postUri, presence=getPresence()]{
            if (!presence)
                return;

            mPostUriRecordUriMap.erase(postUri);
            mPostUriIndex.erase(postUri);
            auto it = std::find(mBookmarkedPostUris.begin(), mBookmarkedPostUris.end(), postUri);

            if (it != mBookmarkedPostUris.end())
                mBookmarkedPostUris.erase(it);

            qDebug() << "Removed bookmark:" << postUri;
            emit sizeChanged();
        },
        [this, atUri, presence=getPresence()](const QString& error, const QString& msg){
            if (!presence)
                return;

            qWarning() << "Failed to delete record:" << atUri.toString() << error << "-" << msg;
            mSkywalker->showStatusMessage(tr("Could not remove bookmark: %1").arg(msg), QEnums::STATUS_LEVEL_ERROR);
        });
}

void Bookmarks::listRecords(const std::function<void()>& doneCb, std::optional<QString> cursor, int maxPages)
{
    qDebug() << "Load bookmarks page, cursor:" << (cursor ? *cursor : "") << "max pages:" << maxPages;

    if (!bskyClient())
        return;

    const QString& repo = mSkywalker->getUserDid();

    bskyClient()->listRecords(repo, Lexicon::COLLECTION_BOOKMARK, 100, cursor,
        [this, doneCb, maxPages, presence=getPresence()](auto output) {
            if (!presence)
                return;

            for (auto it = output->mRecords.rbegin(); it != output->mRecords.rend(); ++it)
            {
                const auto& record = *it;

                try {
                    auto bookmark = Bookmark::Bookmark::fromJson(record->mValue);
                    addBookmarkPrivate(bookmark->mUri);
                    mPostUriRecordUriMap[bookmark->mUri] = record->mUri;
                }
                catch (ATProto::InvalidJsonException& e) {
                    qWarning() << "Record format error:" << record->mUri << e.msg();
                    qInfo() << record->mValue;
                }
            }

            if (output->mCursor && maxPages > 0)
            {
                qDebug() << "Load next page";
                listRecords(doneCb, output->mCursor, maxPages - 1);
            }
            else
            {
                qDebug() << "Bookmarks loaded:" << size();
                emit sizeChanged();
                doneCb();
            }
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to list records:" << error << "-" << msg;
            emit bookmarksLoadFailed(msg);
        });
}

void Bookmarks::createRecords(const QStringList& postUris, const std::function<void()>& doneCb)
{
    ATProto::ComATProtoRepo::ApplyWritesList writes;

    for (const auto& uri : postUris)
    {
        Bookmark::Bookmark record;
        record.mUri = uri;
        auto create = std::make_unique<ATProto::ComATProtoRepo::ApplyWritesCreate>();
        create->mCollection = Lexicon::COLLECTION_BOOKMARK;
        create->mValue = record.toJson();
        writes.push_back(std::move(create));
    }

    const QString& repo = mSkywalker->getUserDid();

    bskyClient()->applyWrites(repo, writes, false,
        [doneCb, presence=getPresence()] {
            if (!presence)
                return;

            if (doneCb)
                doneCb();
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            if (!presence)
                return;

            qDebug() << "Failed to create records:" << error << "-" << msg;
            emit bookmarksLoadFailed(msg);
        });
}

}
