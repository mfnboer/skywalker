// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QTemporaryFile>

namespace Skywalker {

class TempFileHolder
{
public:
    static TempFileHolder& instance();

    ~TempFileHolder();
    void put(std::unique_ptr<QTemporaryFile> tempFile);
    void remove(const QString& fileName);

private:
    std::unordered_map<QString, std::unique_ptr<QTemporaryFile>> mFiles;

    static std::unique_ptr<TempFileHolder> sInstance;
};

}
