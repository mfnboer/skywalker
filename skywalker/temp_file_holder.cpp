// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "temp_file_holder.h"
#include <QDebug>

namespace Skywalker {

std::unique_ptr<TempFileHolder> TempFileHolder::sInstance;

TempFileHolder& TempFileHolder::instance()
{
    if (!sInstance)
        sInstance = std::make_unique<TempFileHolder>();

    return *sInstance;
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

}
