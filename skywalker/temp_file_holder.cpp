// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "temp_file_holder.h"
#include "file_utils.h"
#include <QDebug>
#include <QDir>

namespace Skywalker {

static constexpr const char* TMP_SUB_PATH = "tmp";
static constexpr const char* TMP_FILENAME_PREFIX = "sw_tmp_";

std::unique_ptr<TempFileHolder> TempFileHolder::sInstance;
QString TempFileHolder::sTempPath(".");
QString TempFileHolder::sCachePath(".");

TempFileHolder& TempFileHolder::instance()
{
    if (!sInstance)
        sInstance = std::make_unique<TempFileHolder>();

    return *sInstance;
}

void TempFileHolder::init()
{
    initTempDir();
    initCacheDir();
}

void TempFileHolder::initTempDir()
{
    sTempPath = FileUtils::getAppDataPath(TMP_SUB_PATH);

    if (sTempPath.isEmpty())
        sTempPath = ".";

    qDebug() << "Temp dir:" << sTempPath;
    removeAllFiles(sTempPath);
}

void TempFileHolder::initCacheDir()
{
    sCachePath = FileUtils::getCachePath(TMP_SUB_PATH);

    if (sCachePath.isEmpty())
        sCachePath = ".";

    qDebug() << "Cache dir:" << sCachePath;
    removeAllFiles(sCachePath);
}

void TempFileHolder::removeAllFiles(const QString& path)
{
    qDebug() << "Remove all files in:" << path;

    QDir tmpDir(path);
    const QString tmpFilePattern = QString("%1*").arg(TMP_FILENAME_PREFIX);
    const auto tmpFiles = tmpDir.entryList({tmpFilePattern});

    for (const auto& tmpFile : tmpFiles)
    {
        qDebug() << "Remove temp file:" << tmpFile;
        tmpDir.remove(tmpFile);
    }
}

QString TempFileHolder::getNameTemplate(const QString& fileExtension, bool cache)
{
    return QString("%1/%2XXXXXX.%3").arg(
        cache ? sCachePath : sTempPath,
        TMP_FILENAME_PREFIX,
        fileExtension);
}

QString TempFileHolder::namePrefix()
{
    return TMP_FILENAME_PREFIX;
}

TempFileHolder::~TempFileHolder()
{
    Q_ASSERT(mFiles.empty());
}

void TempFileHolder::put(std::unique_ptr<QTemporaryFile> tempFile)
{
    qDebug() << "Add temp file:" << tempFile->fileName();
    mFiles[tempFile->fileName()] = std::move(tempFile);
}

void TempFileHolder::put(const QString& fileName)
{
    qDebug() << "Add file name:" << fileName;
    mFileNames.insert(fileName);
}

void TempFileHolder::remove(const QString& fileName)
{
    qDebug() << "Remove temp file:" << fileName;
    mFiles.erase(fileName);

    if (mFileNames.contains(fileName))
    {
        qDebug() << "Delete file:" << fileName;
        QFile::remove(fileName);
        mFileNames.erase(fileName);
    }
}

bool TempFileHolder::contains(const QString& fileName) const
{
    return mFiles.contains(fileName) || mFileNames.contains(fileName);
}

}
