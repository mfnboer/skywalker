// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QTemporaryFile>
#include <unordered_set>

namespace Skywalker {

class TempFileHolder
{
public:
    static TempFileHolder& instance();
    static void init();
    static QString getNameTemplate(const QString& fileExtension, bool cache);
    static QString namePrefix();

    ~TempFileHolder();
    void put(std::unique_ptr<QTemporaryFile> tempFile);
    void put(const QString& fileName);
    void remove(const QString& fileName);
    bool contains(const QString& fileName) const;

private:
    static void initTempDir();
    static void initCacheDir();
    static void removeAllFiles(const QString& path);

    std::unordered_map<QString, std::unique_ptr<QTemporaryFile>> mFiles;

    // These are temp files that are not created with QTemporaryFile
    std::unordered_set<QString> mFileNames;

    static std::unique_ptr<TempFileHolder> sInstance;
    static QString sTempPath;
    static QString sCachePath;
};

}
