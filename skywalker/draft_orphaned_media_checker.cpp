// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "draft_orphaned_media_checker.h"
#include "draft_posts.h"
#include "utils.h"

namespace Skywalker {

DraftOrphanedMediaChecker::DraftOrphanedMediaChecker(const QString& userDid, ATProto::Client::SharedPtr& bsky) :
    mUserDid(userDid),
    mBsky(bsky)
{
    qDebug() << "Created draft orphaned media checker:" << mUserDid;
}

DraftOrphanedMediaChecker::~DraftOrphanedMediaChecker()
{
    qDebug() << "Destroyed draft orphaned media checker:" << mUserDid;
}

void DraftOrphanedMediaChecker::start(const std::function<void()>& finishedCb)
{
    qDebug() << "Start draft orphaned media check:" << mUserDid;
    Q_ASSERT(mBsky);

    if (!mBsky)
        return;

    const QStringList fileNames = getMediaFileNames();

    if (fileNames.empty())
    {
        qDebug() << "No draft media files";

        if (finishedCb)
            QTimer::singleShot(0, finishedCb);

        return;
    }

    qDebug() << "Media files:" << fileNames.size() << "names:" << fileNames;
    mFileNamesToCheck.insert(fileNames.begin(), fileNames.end());
    mFinishedCb = finishedCb;
    checkMediaFiles();
}

void DraftOrphanedMediaChecker::checkMediaFiles(int maxPages, const QString& cursor)
{
    qDebug() << "Check media files, maxPages:" << maxPages << "cursor:" << cursor;

    if (maxPages <= 0)
    {
        qWarning() << "Too many pages to check!";
        callFinishCb();
        return;
    }

    mBsky->getDrafts(25, Utils::makeOptionalString(cursor),
        [this, presence=getPresence(), maxPages](ATProto::AppBskyDraft::GetDraftsOutput::SharedPtr output){
            if (!presence)
                return;

            updateMediaFiles(output->mDrafts);

            if (mFileNamesToCheck.empty())
            {
                qDebug() << "No orphaned media files.";
                callFinishCb();
                return;
            }

            if (output->mCursor && !output->mCursor->isEmpty())
            {
                checkMediaFiles(maxPages - 1, *output->mCursor);
            }
            else
            {
                cleanupOrphans();
                callFinishCb();
            }
        },
        [this, presence=getPresence()](const QString& error, const QString& msg) {
            qWarning() << "Failed to get drafts:" << error << "-" << msg;

            if (!presence)
                return;

            callFinishCb();
        });
}

void DraftOrphanedMediaChecker::updateMediaFiles(const ATProto::AppBskyDraft::DraftView::List& drafts)
{
    const QString mediaPath = getPictureDraftsPath();

    if (mediaPath.isEmpty())
    {
        qWarning() << "Failed to get media path, stop checking";
        mFileNamesToCheck.clear();
    }

    for (const auto& draft : drafts)
    {
        for (const auto& draftPost : draft->mDraft->mPosts)
        {
            updateMediaFiles(draftPost);

            if (mFileNamesToCheck.empty())
                return;
        }
    }
}

void DraftOrphanedMediaChecker::updateMediaFiles(const ATProto::AppBskyDraft::DraftPost::SharedPtr& draftPost)
{
    for (const auto& embedImage : draftPost->mEmbedImages)
    {
        const QString fileName = embedImage->mLocalRef->mPath;
        qDebug() << "Found file:" << fileName;
        mFileNamesToCheck.erase(fileName);
    }

    for (const auto& embedVideo : draftPost->mEmbedVideos)
    {
        const QString fileName = embedVideo->mLocalRef->mPath;
        qDebug() << "Found file:" << fileName;
        mFileNamesToCheck.erase(fileName);
    }

    qDebug() << "File names left:" << mFileNamesToCheck.size();
}

QString DraftOrphanedMediaChecker::getPictureDraftsPath() const
{
    return DraftPosts::getPictureDraftsPath(DraftPosts::STORAGE_BLUESKY, mUserDid);
}

QStringList DraftOrphanedMediaChecker::getMediaFileNames() const
{
    const QString mediaPath = getPictureDraftsPath();

    if (mediaPath.isEmpty())
        return {};

    QDir mediaDir(mediaPath);
    const auto imgFileNames = mediaDir.entryList({ "SWI*_*-*.jpg" }, QDir::Files);
    const auto videoFileNames = mediaDir.entryList({ "SWV*_*-*.mp4" }, QDir::Files);
    const QStringList mediaFileNames = imgFileNames + videoFileNames;
    return mediaFileNames;
}

void DraftOrphanedMediaChecker::cleanupOrphans()
{
    qDebug() << "Cleanup orphans:" << mFileNamesToCheck;

    const QString mediaPath = getPictureDraftsPath();

    if (mediaPath.isEmpty())
        return;

    QDir dir(mediaPath);

    for (const auto& fileName : mFileNamesToCheck)
    {
        if (dir.remove(fileName))
            qDebug() << "Removed orphaned media file:" << fileName << "in dir:" << mediaPath;
        else
            qWarning() << "Failed to remove orphaned media file:" << fileName << "in dir:" << mediaPath;
    }

    mFileNamesToCheck.clear();
}

void DraftOrphanedMediaChecker::callFinishCb()
{
    if (mFinishedCb)
    {
        auto cb = mFinishedCb;
        mFinishedCb = {};
        cb();
    }
}

}
