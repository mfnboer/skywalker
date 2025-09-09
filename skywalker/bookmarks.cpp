// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "bookmarks.h"
#include "bookmarks_model.h"
#include "skywalker.h"
#include "utils.h"

namespace Skywalker {

static constexpr int ADD_PAGE_SIZE = 100;

Bookmarks::Bookmarks(QObject* parent) :
    WrappedSkywalker(parent)
{
}

void Bookmarks::getBookmarks(int limit, const QString& cursor)
{
    qDebug() << "Get bookmarks, limit:" << limit << "cursor:" << cursor;
    BookmarksModel* model = mSkywalker->getBookmarksModel();

    if (!model)
    {
        qWarning() << "No bookmarks model available";
        return;
    }

    if (model->isGetFeedInProgress())
    {
        qDebug() << "Get bookmarks still in progress";
        return;
    }

    model->setGetFeedInProgress(true);

    bskyClient()->getBookmarks(limit, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), cursor](auto output){
            if (!presence)
                return;

            BookmarksModel* model = mSkywalker->getBookmarksModel();

            if (!model)
            {
                qWarning() << "No bookmarks model available";
                return;
            }

            model->setGetFeedInProgress(false);

            if (cursor.isEmpty())
                model->clear();

            model->addBookmarks(*output);
        },
        [this, presence=getPresence()](const QString& error, const QString& msg){
            qWarning() << "getBookmarks failed:" << error << "-" << msg;

            if (!presence)
                return;

            BookmarksModel* model = mSkywalker->getBookmarksModel();

            if (!model)
            {
                qWarning() << "No bookmarks model available";
                return;
            }

            model->setGetFeedInProgress(false);
            model->setFeedError(msg);

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        }
    );
}

void Bookmarks::getBookmarksNextPage()
{
    BookmarksModel* model = mSkywalker->getBookmarksModel();

    if (!model)
    {
        qWarning() << "No bookmarks model available";
        return;
    }

    const QString& cursor = model->getCursorNextPage();
    if (cursor.isEmpty())
    {
        qDebug() << "Last page reached, no more cursor";
        return;
    }

    getBookmarks(ADD_PAGE_SIZE, cursor);
}

void Bookmarks::addBookmark(const QString& postUri, const QString& postCid)
{
    qDebug() << "Add bookmark:" << postUri << "cid:" << postCid;

    mSkywalker->makeLocalModelChange(
        [postCid](LocalPostModelChanges* model){
            model->updateBookmarkTransient(postCid, true);
        });

    bskyClient()->createBookmark(postUri, postCid,
        [this, presence=getPresence(), postUri, postCid]{
            qDebug() << "Bookmark added:" << postUri << "cid:" << postCid;

            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [postUri, postCid](LocalPostModelChanges* model){
                    model->updateBookmarked(postCid, true);
                    model->updateBookmarkTransient(postCid, false);
                });
        },
        [this, presence=getPresence(), postCid](const QString& error, const QString& msg){
            qWarning() << "Add bookmark failed:" << error << "-" << msg;

            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [postCid](LocalPostModelChanges* model){
                    model->updateBookmarkTransient(postCid, false);
                });
        });
}

void Bookmarks::removeBookmark(const QString& postUri, const QString& postCid)
{
    qDebug() << "Add bookmark:" << postUri << "cid:" << postCid;

    mSkywalker->makeLocalModelChange(
        [postCid](LocalPostModelChanges* model){
            model->updateBookmarkTransient(postCid, true);
        });

    bskyClient()->deleteBookmark(postUri,
        [this, presence=getPresence(), postUri, postCid]{
            qDebug() << "Bookmark removed:" << postUri << "cid:" << postCid;

            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [postUri, postCid](LocalPostModelChanges* model){
                    model->updateBookmarked(postCid, false);
                    model->updateBookmarkTransient(postCid, false);
                });
        },
        [this, presence=getPresence(), postCid](const QString& error, const QString& msg){
            qWarning() << "Remove bookmark failed:" << error << "-" << msg;

            if (!presence)
                return;

            mSkywalker->makeLocalModelChange(
                [postCid](LocalPostModelChanges* model){
                    model->updateBookmarkTransient(postCid, false);
                });
        });
}

bool Bookmarks::migrateToBsky()
{
    qDebug() << "Migrate local bookmarks to bsky";
    mFailedToMigrateUris.clear();
    mSucceededToMigrateUris.clear();
    Q_ASSERT(mSkywalker);
    UserSettings* settings = mSkywalker->getUserSettings();
    Q_ASSERT(settings);

    QStringList postUris = settings->getBookmarks(mSkywalker->getUserDid());

    if (postUris.empty())
    {
        qDebug() << "No migration needed";
        return false;
    }

    if (settings->getBookmarksMigrationAttempts(mSkywalker->getUserDid()) >= 8)
    {
        qWarning() << "Maximum migration attempts reached";
        return false;
    }

    setToMigrateCount(postUris.size());
    setMigratedCount(0);
    setMigrationInProgress(true);
    createBookmarks(postUris, 0);
    return true;
}

void Bookmarks::createBookmarks(const QStringList& postUris, int startIndex)
{
    qDebug() << "Create bookmarks, size:" << postUris.size() << "index:" << startIndex;

    if (startIndex >= postUris.size())
    {
        qDebug() << "Local bookmarks migrated, failed:" << mFailedToMigrateUris;
        UserSettings* settings = mSkywalker->getUserSettings();
        Q_ASSERT(settings);
        settings->setBookmarks(mSkywalker->getUserDid(), mFailedToMigrateUris);
        setMigrationInProgress(false);

        const int attempts = settings->getBookmarksMigrationAttempts(mSkywalker->getUserDid());
        settings->setBookmarksMigrationAttempts(mSkywalker->getUserDid(), attempts + 1);

        if (!mFailedToMigrateUris.empty())
            mSkywalker->showStatusMessage(tr("Failed to migrate %1 bookmarks. Will retry later.").arg(mFailedToMigrateUris.size()), QEnums::STATUS_LEVEL_ERROR);

        return;
    }

    const int chunkSize = std::min(ATProto::Client::MAX_URIS_GET_POSTS, (int)postUris.size() - startIndex);
    std::vector<QString> uris(postUris.begin() + startIndex, postUris.begin() + startIndex + chunkSize);

    bskyClient()->getPosts(uris,
        [this, presence=getPresence(), postUris, startIndex, chunkSize](const ATProto::AppBskyFeed::PostView::List& postViewList){
            if (!presence)
                return;

            createBookmarks(postViewList, 0,
                [this, postUris, startIndex, chunkSize]{
                    createBookmarks(postUris, startIndex + chunkSize);
                });
        },
        [this, presence=getPresence(), uris, postUris, startIndex, chunkSize](const QString& error, const QString& msg)
        {
            qWarning() << "Failed to get posts:" << uris << "error:" << error << " - " << msg;

            if (!presence)
                return;

            for (const auto& uri : uris)
                mFailedToMigrateUris.push_back(uri);

            createBookmarks(postUris, startIndex + chunkSize);
        });
}

void Bookmarks::createBookmarks(const ATProto::AppBskyFeed::PostView::List& postViewList, int index, const std::function<void()>& doneCb)
{
    qDebug() << "Create bookmarks post view list:" << postViewList.size() << "index:" << index;

    if (index >= (int)postViewList.size())
    {
        qDebug() << "Created all bookmarks for post view list";
        updateLocalBookmarks();
        doneCb();
        return;
    }

    const auto& postView = postViewList[index];
    const auto& postUri = postView->mUri;
    const auto& postCid = postView->mCid;

    bskyClient()->createBookmark(postUri, postCid,
        [this, presence=getPresence(), postUri, postCid, postViewList, index, doneCb]{
            qDebug() << "Bookmark created:" << postUri << "cid:" << postCid;

            if (!presence)
                return;

            mSucceededToMigrateUris.insert(postUri);
            setMigratedCount(mMigratedCount + 1);
            createBookmarks(postViewList, index + 1, doneCb);
        },
        [this, presence=getPresence(), postUri, postCid, postViewList, index, doneCb](const QString& error, const QString& msg){
            qWarning() << "Create bookmark failed:" << postUri << "cid:" << postCid << "error:" << error << "-" << msg;

            if (!presence)
                return;

            mFailedToMigrateUris.push_back(postUri);
            createBookmarks(postViewList, index + 1, doneCb);
        });
}

void Bookmarks::updateLocalBookmarks()
{
    Q_ASSERT(mSkywalker);
    UserSettings* settings = mSkywalker->getUserSettings();
    Q_ASSERT(settings);
    const QStringList postUris = settings->getBookmarks(mSkywalker->getUserDid());
    QStringList toSaveUris;
    toSaveUris.reserve(postUris.size());

    for (const auto& uri: postUris)
    {
        if (!mSucceededToMigrateUris.contains(uri))
            toSaveUris.push_back(uri);
    }

    qDebug() << "Update local bookmarks:" << toSaveUris.size();
    settings->setBookmarks(mSkywalker->getUserDid(), toSaveUris);
}

void Bookmarks::setMigrationInProgress(bool inProgress)
{
    if (inProgress != mMigrationInProgress)
    {
        mMigrationInProgress = inProgress;
        emit migrationInProgressChanged();
    }
}

void Bookmarks::setToMigrateCount(int count)
{
    if (count != mToMigrateCount)
    {
        mToMigrateCount = count;
        emit toMigrateCountChanged();
    }
}

void Bookmarks::setMigratedCount(int count)
{
    if (count != mMigratedCount)
    {
        mMigratedCount = count;
        emit migratedCountChanged();
    }
}

}
