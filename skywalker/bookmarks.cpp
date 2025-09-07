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

            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
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


            mSkywalker->showStatusMessage(msg, QEnums::STATUS_LEVEL_ERROR);
        });
}

}
