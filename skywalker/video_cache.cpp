// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "video_cache.h"
#include "temp_file_holder.h"
#include <qdebug.h>

namespace Skywalker {

VideoHandle::VideoHandle(QObject* parent) : QObject(parent)
{
}

VideoHandle::VideoHandle(const QString& link, const QString& fileName, QObject* parent) :
    QObject(parent),
    mLink(link),
    mFileName(fileName)
{
    qDebug() << "Create video handle:" << link << "file:" << fileName;
}

VideoHandle::~VideoHandle()
{
    if (isValid()) {
        qDebug() << "Destructor video handle:" << mLink << "file:" << mFileName;
        VideoCache::instance().unlinkVideo(mLink, mFileName);
    }
}


std::unique_ptr<VideoCache> VideoCache::sInstance;

VideoCache& VideoCache::instance()
{
    if (!sInstance)
        sInstance = std::make_unique<VideoCache>();

    return *sInstance;
}

VideoHandle* VideoCache::putVideo(const QString& link, const QString& fileName)
{
    qDebug() << "Put video:" << link << "file:" << fileName;

    if (!TempFileHolder::instance().contains(fileName))
    {
        qWarning() << "Video not a held tmp file:" << fileName;
        TempFileHolder::instance().put(fileName);
    }

    auto* handle = getVideo(link);

    if (handle->isValid())
    {
        qDebug() << "File already stored:" << fileName << "existing:" << handle->getFileName();

        if (fileName != handle->getFileName())
            TempFileHolder::instance().remove(fileName);

        return handle;
    }

    delete handle;

    CacheEntry entry;
    entry.mFileName = fileName;
    entry.mCount = 1;

    qDebug() << "Put video:" << link << "file:" << entry.mFileName << "count:" << entry.mCount;
    mCache[link] = entry;

    qDebug() << "Cache size:" << mCache.size();
    return new VideoHandle(link, fileName);
}

VideoHandle* VideoCache::getVideo(const QString& link)
{
    if (!mCache.contains(link))
        return new VideoHandle();

    auto& entry = mCache[link];

    if (!QFile::exists(entry.mFileName))
    {
        // This can happen when a file was forcefully deleted from cache.
        qWarning() << "Get video, file does not exist:" << link << "file:" << entry.mFileName << "count:" << entry.mCount;
        TempFileHolder::instance().remove(entry.mFileName);
        mCache.erase(link);
        return new VideoHandle();
    }

    ++entry.mCount;
    qDebug() << "Get video:" << link << "file:" << entry.mFileName << "count:" << entry.mCount;
    return new VideoHandle(link, entry.mFileName);
}

void VideoCache::unlinkVideo(const QString& link, const QString& fileName)
{
    if (!mCache.contains(link))
    {
        qDebug() << "Unlink video, does not exist:" << link;
        return;
    }

    auto& entry = mCache[link];

    if (entry.mFileName != fileName)
    {
        // This can happen when a file was forcefully deleted from cache.
        qWarning() << "Link file names do not match:" << fileName << "cached:" << entry.mFileName;
        return;
    }

    --entry.mCount;
    qDebug() << "Unlink video:" << link << "file:" << entry.mFileName << "count:" << entry.mCount;

    if (entry.mCount <= 0)
    {
        qDebug() << "Delete video:" << link << "file:" << entry.mFileName << "count:" << entry.mCount;
        TempFileHolder::instance().remove(entry.mFileName);
        mCache.erase(link);
    }

    qDebug() << "Cache size:" << mCache.size();
}

}
